#include <boost/algorithm/string/predicate.hpp>

#include "WalletRpcServer.h"
#include "crypto/hash.h"
#include "common/CommandLine.h"
#include "common/StringTools.h"
#include "base/CryptoNoteFormatUtils.h"
#include "core/Account.h"
#include "rpc/JsonRpc.h"
#include "wallet_legacy/WalletHelper.h"
#include "wallet_legacy/WalletLegacy.h"
#include "common/StringTools.h"
#include "common/Base58.h"
#include "common/Util.h"

using namespace Logging;
using namespace CryptoNote;

namespace Tools {

const command_line::arg_descriptor<uint16_t>    wallet_rpc_server::arg_rpc_bind_port = 
	{ "rpc-bind-port", "Starts wallet as RPC server for wallet operations, sets bind port for server.", 0, true };
const command_line::arg_descriptor<std::string> wallet_rpc_server::arg_rpc_bind_ip = 
	{ "rpc-bind-ip"  , "Specify IP to bind RPC server to.", "127.0.0.1" };
const command_line::arg_descriptor<std::string> wallet_rpc_server::arg_rpc_user = 
	{ "rpc-user"     , "Username to use with the RPC server. If empty, no server authorization will be done.", "" };
const command_line::arg_descriptor<std::string> wallet_rpc_server::arg_rpc_password = 
	{ "rpc-password" , "Password to use with the RPC server. If empty, no server authorization will be done.", "" };

void wallet_rpc_server::init_options(boost::program_options::options_description& desc) {
	command_line::add_arg(desc, arg_rpc_bind_ip);
	command_line::add_arg(desc, arg_rpc_bind_port);
	command_line::add_arg(desc, arg_rpc_user);
	command_line::add_arg(desc, arg_rpc_password);
}
//------------------------------------------------------------------------------------------------------------------------------
wallet_rpc_server::wallet_rpc_server(
  System::Dispatcher& dispatcher, 
  Logging::ILogger& log, 
  CryptoNote::IWalletLegacy&w,
  CryptoNote::INode& n, 
  CryptoNote::Currency& currency, 
  const std::string& walletFile) : 
  HttpServer(dispatcher, log), 
  logger(log, "WalletRpc"), 
  m_dispatcher(dispatcher), 
  m_stopComplete(dispatcher), 
  m_wallet(w),
  m_node(n), 
  m_currency(currency),
  m_walletFilename(walletFile) 
{}
//------------------------------------------------------------------------------------------------------------------------------
bool wallet_rpc_server::run() {
  start(m_bind_ip, m_port, m_rpcUser, m_rpcPassword);
  m_stopComplete.wait();
  return true;
}
//------------------------------------------------------------------------------------------------------------------------------
void wallet_rpc_server::send_stop_signal() {
  m_dispatcher.remoteSpawn([this] {
    std::cout << "wallet_rpc_server::send_stop_signal()" << std::endl;
    stop();
    m_stopComplete.set();
  });
}
//------------------------------------------------------------------------------------------------------------------------------
bool wallet_rpc_server::handle_command_line(const boost::program_options::variables_map& vm) {
  m_bind_ip = command_line::get_arg(vm, arg_rpc_bind_ip);
  m_port = command_line::get_arg(vm, arg_rpc_bind_port);
  m_rpcUser = command_line::get_arg(vm, arg_rpc_user);
  m_rpcPassword = command_line::get_arg(vm, arg_rpc_password);
  return true;
}
//------------------------------------------------------------------------------------------------------------------------------
bool wallet_rpc_server::init(const boost::program_options::variables_map& vm) {
  if (!handle_command_line(vm)) {
    logger(ERROR) << "Failed to process command line in wallet_rpc_server";
    return false;
  }

  return true;
}

void wallet_rpc_server::processRequest(const CryptoNote::HttpRequest& request, CryptoNote::HttpResponse& response) {

  using namespace CryptoNote::JsonRpc;

  JsonRpcRequest jsonRequest;
  JsonRpcResponse jsonResponse;

  try {
    jsonRequest.parseRequest(request.getBody());
    jsonResponse.setId(jsonRequest.getId());
    logger(INFO) << "ASSEM5: json=" << request.getBody();
    static std::unordered_map<std::string, JsonMemberMethod> s_methods = {
      { "getbalance", makeMemberMethod(&wallet_rpc_server::on_getbalance) },
      { "transfer", makeMemberMethod(&wallet_rpc_server::on_transfer) },
      { "store", makeMemberMethod(&wallet_rpc_server::on_store) },
      { "stop_wallet" , makeMemberMethod(&wallet_rpc_server::on_stop_wallet) },
      { "get_payments", makeMemberMethod(&wallet_rpc_server::on_get_payments) },
      { "get_transfers", makeMemberMethod(&wallet_rpc_server::on_get_transfers) },
      //{ "get_transaction", makeMemberMethod(&wallet_rpc_server::on_get_transaction) },
      { "get_height", makeMemberMethod(&wallet_rpc_server::on_get_height) },
      { "get_address", makeMemberMethod(&wallet_rpc_server::on_get_address) },
      { "query_key" , makeMemberMethod(&wallet_rpc_server::on_query_key) },
      { "reset", makeMemberMethod(&wallet_rpc_server::on_reset) },
      { "get_paymentid" , makeMemberMethod(&wallet_rpc_server::on_gen_paymentid) },
    };

    auto it = s_methods.find(jsonRequest.getMethod());
    if (it == s_methods.end()) {
      throw JsonRpcError(errMethodNotFound);
    }

    it->second(this, jsonRequest, jsonResponse);

  } catch (const JsonRpcError& err) {
    jsonResponse.setError(err);
  } catch (const std::exception& e) {
    jsonResponse.setError(JsonRpcError(WALLET_RPC_ERROR_CODE_UNKNOWN_ERROR, e.what()));
  }

  response.setBody(jsonResponse.getBody());
}

//------------------------------------------------------------------------------------------------------------------------------
bool wallet_rpc_server::on_getbalance(const wallet_rpc::COMMAND_RPC_GET_BALANCE::request& req, wallet_rpc::COMMAND_RPC_GET_BALANCE::response& res) {
  res.locked_amount = m_wallet.pendingBalance();
  res.available_balance = m_wallet.actualBalance();
  res.balance = res.locked_amount + res.available_balance;
  res.unlocked_balance = res.available_balance;
  return true;
}
//------------------------------------------------------------------------------------------------------------------------------
bool wallet_rpc_server::on_transfer(const wallet_rpc::COMMAND_RPC_TRANSFER::request& req, wallet_rpc::COMMAND_RPC_TRANSFER::response& res) {
  std::vector<CryptoNote::WalletLegacyTransfer> transfers;
  std::vector<CryptoNote::TransactionMessage> messages;
  for (auto it = req.destinations.begin(); it != req.destinations.end(); it++) {
    CryptoNote::WalletLegacyTransfer transfer;
    transfer.address = it->address;
    transfer.amount = it->amount;
    transfers.push_back(transfer);

    if (!it->message.empty()) {
      messages.emplace_back(CryptoNote::TransactionMessage{ it->message, it->address });
    }
  }
  logger(INFO) << "ASSEM3: kribbz_infoe=" << req.kribbz_info;
  logger(INFO) << "ASSEM4: req.payment_id=" << req.payment_id;

  std::vector<uint8_t> extra;
  if (!req.payment_id.empty()) {
    std::string payment_id_str = req.payment_id;

    Crypto::Hash payment_id;
    if (!CryptoNote::parsePaymentId(payment_id_str, payment_id)) {
      throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_WRONG_PAYMENT_ID, 
        "Payment id has invalid format: \"" + payment_id_str + "\", expected 64-character string");
    }

    BinaryArray extra_nonce;
    CryptoNote::setPaymentIdToTransactionExtraNonce(extra_nonce, payment_id);
    if (!CryptoNote::addExtraNonceToTransactionExtra(extra, extra_nonce)) {
      throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_WRONG_PAYMENT_ID,
        "Something went wrong with payment_id. Please check its format: \"" + payment_id_str + "\", expected 64-character string");
    }
  }

  
    const char *buff = req.kribbz_info.c_str();
    int len = req.kribbz_info.length();
    std::vector<uint8_t> kribbz_value(len + 1);
    memcpy((void*) &kribbz_value[0], buff, len);
    logger(INFO) << " kribbz_value=" << Common::podToHex(kribbz_value);
    logger(DEBUGGING, BRIGHT_RED) << "assem 2 size: " << kribbz_value.size();
    if (!createTxExtraKribbz(kribbz_value, extra)) {
        logger(ERROR, BRIGHT_RED) << "kribbz invalid format";
        throw std::runtime_error("Kribbz invalid format");
    }
    logger(INFO) << " extra2=" << Common::podToHex( extra);
  
  std::string extraString;
  std::copy(extra.begin(), extra.end(), std::back_inserter(extraString));
  try {
    CryptoNote::WalletHelper::SendCompleteResultObserver sent;
    WalletHelper::IWalletRemoveObserverGuard removeGuard(m_wallet, sent);

    CryptoNote::TransactionId tx = m_wallet.sendTransaction(transfers, req.fee, extraString, req.mixin, req.unlock_time);
    if (tx == WALLET_LEGACY_INVALID_TRANSACTION_ID) {
      throw std::runtime_error("Couldn't send transaction");
    }

    removeGuard.removeObserver();

    CryptoNote::WalletLegacyTransaction txInfo;
    m_wallet.getTransaction(tx, txInfo);
    res.tx_hash = Common::podToHex(txInfo.hash);

  } catch (const std::exception& e) {
    throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_GENERIC_TRANSFER_ERROR, e.what());
  }
  return true;
}
//------------------------------------------------------------------------------------------------------------------------------
bool wallet_rpc_server::on_store(const wallet_rpc::COMMAND_RPC_STORE::request& req, wallet_rpc::COMMAND_RPC_STORE::response& res) {
  try {
    res.stored = WalletHelper::storeWallet(m_wallet, m_walletFilename);
  } catch (std::exception& e) {
    throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_UNKNOWN_ERROR, std::string("Couldn't save wallet: ") + e.what());
    return false;
  }

  return true;
}
//------------------------------------------------------------------------------------------------------------------------------
bool wallet_rpc_server::on_get_payments(const wallet_rpc::COMMAND_RPC_GET_PAYMENTS::request& req, wallet_rpc::COMMAND_RPC_GET_PAYMENTS::response& res) {
  Crypto::Hash expectedPaymentId;
	CryptoNote::BinaryArray payment_id_blob;

	if (!Common::fromHex(req.payment_id, payment_id_blob))
		throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_WRONG_PAYMENT_ID, "Payment ID has invald format");
	if (sizeof(expectedPaymentId) != payment_id_blob.size())
		throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_WRONG_PAYMENT_ID, "Payment ID has invalid size");

	expectedPaymentId = *reinterpret_cast<const Crypto::Hash*>(payment_id_blob.data());
	size_t transactionsCount = m_wallet.getTransactionCount();
	for (size_t trantransactionNumber = 0; trantransactionNumber < transactionsCount; ++trantransactionNumber)
	{
		WalletLegacyTransaction txInfo;
		m_wallet.getTransaction(trantransactionNumber, txInfo);
		if (txInfo.state != WalletLegacyTransactionState::Active || txInfo.blockHeight == WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT)
			continue;
		if (txInfo.totalAmount < 0)
			continue;
		std::vector<uint8_t> extraVec;
		extraVec.reserve(txInfo.extra.size());
		std::for_each(txInfo.extra.begin(), txInfo.extra.end(), 
			[&extraVec](const char el) { extraVec.push_back(el); });

		Crypto::Hash paymentId;
		if (getPaymentIdFromTxExtra(extraVec, paymentId) && paymentId == expectedPaymentId)
		{
			wallet_rpc::payment_details rpc_payment;
			rpc_payment.tx_hash      = Common::podToHex(txInfo.hash);
			rpc_payment.amount       = txInfo.totalAmount;
			rpc_payment.block_height = txInfo.blockHeight;
			rpc_payment.unlock_time  = txInfo.unlockTime;
			res.payments.push_back(rpc_payment);
		}
	}
return true;
}
//------------------------------------------------------------------------------------------------------------------------------
bool wallet_rpc_server::on_get_transfers(const wallet_rpc::COMMAND_RPC_GET_TRANSFERS::request& req, 
	wallet_rpc::COMMAND_RPC_GET_TRANSFERS::response& res)
{
	res.transfers.clear();
	size_t transactionsCount = m_wallet.getTransactionCount();
        uint64_t bc_height;
	try {
		bc_height = m_node.getKnownBlockCount();
	} catch (std::exception &e) {
		throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_UNKNOWN_ERROR, std::string("Failed to get blockchain height: ") + e.what());
	}
	for (size_t trantransactionNumber = 0; trantransactionNumber < transactionsCount; ++trantransactionNumber)
	{
		WalletLegacyTransaction txInfo;
		m_wallet.getTransaction(trantransactionNumber, txInfo);
		//if (txInfo.state != WalletLegacyTransactionState::Sending || txInfo.state != WalletLegacyTransactionState::Active || txInfo.blockHeight == WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT)
		if (txInfo.state == WalletLegacyTransactionState::Cancelled || txInfo.state == WalletLegacyTransactionState::Deleted || txInfo.state == WalletLegacyTransactionState::Failed)
			continue;

		std::string address = "";
		if (txInfo.totalAmount < 0 && txInfo.transferCount > 0)
		{
			WalletLegacyTransfer tr;
			m_wallet.getTransfer(txInfo.firstTransferId, tr);
			address = tr.address;
		}

		wallet_rpc::Transfer transfer;
		transfer.time			 = txInfo.timestamp;
		transfer.output			 = txInfo.totalAmount < 0;
		transfer.transactionHash = Common::podToHex(txInfo.hash);
		transfer.amount			 = std::abs(txInfo.totalAmount);
		transfer.fee			 = txInfo.fee;
		transfer.address		 = address;
		transfer.blockIndex		 = txInfo.blockHeight;
		transfer.unlockTime		 = txInfo.unlockTime;
		transfer.paymentId		 = "";
                transfer.confirmations = bc_height - txInfo.blockHeight;

		std::vector<uint8_t> extraVec;
		extraVec.reserve(txInfo.extra.size());
		std::for_each(txInfo.extra.begin(), txInfo.extra.end(), [&extraVec](const char el) { extraVec.push_back(el); });

		Crypto::Hash paymentId;
		transfer.paymentId = (getPaymentIdFromTxExtra(extraVec, paymentId) && paymentId != NULL_HASH ? Common::podToHex(paymentId) : "");

		res.transfers.push_back(transfer);
	}
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------
bool wallet_rpc_server::on_get_height(const wallet_rpc::COMMAND_RPC_GET_HEIGHT::request& req, wallet_rpc::COMMAND_RPC_GET_HEIGHT::response& res) {
  res.height = m_node.getLastLocalBlockHeight();
  return true;
}
//------------------------------------------------------------------------------------------------------------------------------
bool wallet_rpc_server::on_get_address(const wallet_rpc::COMMAND_RPC_GET_ADDRESS::request& req, wallet_rpc::COMMAND_RPC_GET_ADDRESS::response& res) {
  res.address = m_wallet.getAddress();
  return true;
}

bool wallet_rpc_server::on_reset(const wallet_rpc::COMMAND_RPC_RESET::request& req, wallet_rpc::COMMAND_RPC_RESET::response& res) {
  m_wallet.reset();
  return true;
}
//------------------------------------------------------------------------------------------------------------------------------
bool wallet_rpc_server::on_stop_wallet(const wallet_rpc::COMMAND_RPC_STOP::request& req, wallet_rpc::COMMAND_RPC_STOP::response& res) {
  try {
     WalletHelper::storeWallet(m_wallet, m_walletFilename);
  } catch (std::exception& e) {
    throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_UNKNOWN_ERROR, std::string("Couldn't save wallet: ") + e.what());
  }
    wallet_rpc_server::send_stop_signal();
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------
/*bool wallet_rpc_server::on_get_transaction(const wallet_rpc::COMMAND_RPC_GET_TRANSACTION::request& req,
	wallet_rpc::COMMAND_RPC_GET_TRANSACTION::response& res) {
  res.destinations.clear();
  uint64_t bc_height;
  try {
    bc_height = m_node.getKnownBlockCount();
  } catch (std::exception &e) {
    throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_UNKNOWN_ERROR, std::string("Failed to get blockchain height: ") + e.what());
  }

  size_t transactionsCount = m_wallet.getTransactionCount();
  for (size_t transactionNumber = 0; transactionNumber < transactionsCount; ++transactionNumber) {
    WalletLegacyTransaction txInfo;
    m_wallet.getTransaction(transactionNumber, txInfo);
  if (txInfo.state == WalletLegacyTransactionState::Cancelled || txInfo.state == WalletLegacyTransactionState::Deleted || txInfo.state == WalletLegacyTransactionState::Failed)
    continue;

  if (boost::iequals(Common::podToHex(txInfo.hash), req.tx_hash)) {
    std::string address = "";
    if (txInfo.totalAmount < 0 && txInfo.transferCount > 0) {
	WalletLegacyTransfer ftr;
	m_wallet.getTransfer(txInfo.firstTransferId, ftr);
	address = ftr.address;
    }

	wallet_rpc::Transfer transfer;
	transfer.time = txInfo.timestamp;
	transfer.output = txInfo.totalAmount < 0;
	transfer.transactionHash = Common::podToHex(txInfo.hash);
	transfer.amount = std::abs(txInfo.totalAmount);
	transfer.fee = txInfo.fee;
	transfer.address = address;
	transfer.blockIndex = txInfo.blockHeight;
	transfer.unlockTime = txInfo.unlockTime;
	transfer.paymentId = "";
	transfer.confirmations = (txInfo.blockHeight != UNCONFIRMED_TRANSACTION_GLOBAL_OUTPUT_INDEX ? bc_height - txInfo.blockHeight : 0);

	std::vector<uint8_t> extraVec;
	extraVec.reserve(txInfo.extra.size());
	std::for_each(txInfo.extra.begin(), txInfo.extra.end(), [&extraVec](const char el) { extraVec.push_back(el); });

	Crypto::Hash paymentId;
	transfer.paymentId = (getPaymentIdFromTxExtra(extraVec, paymentId) && paymentId != NULL_HASH ? Common::podToHex(paymentId) : "");

	res.transaction_details = transfer;

     for (TransferId id = txInfo.firstTransferId; id < txInfo.firstTransferId + txInfo.transferCount; ++id) {
	WalletLegacyTransfer txtr;
	m_wallet.getTransfer(id, txtr);
	wallet_rpc::transfer_destination dest;
	dest.amount = txtr.amount;
	dest.address = txtr.address;
	res.destinations.push_back(dest);
	}
	return true;
       }
     }

     throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_UNKNOWN_ERROR,
	std::string("Transaction with this hash not found: ") + req.tx_hash);

  return false;
}*/

bool wallet_rpc_server::on_query_key(const wallet_rpc::COMMAND_RPC_QUERY_KEY::request& req,
	wallet_rpc::COMMAND_RPC_QUERY_KEY::response& res) {
   if (0 != req.key_type.compare("mnemonic") && 0 != req.key_type.compare("paperwallet"))
        throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_UNKNOWN_ERROR, std::string("Unsupported key_type ") + req.key_type);
   if (0 == req.key_type.compare("mnemonic") && !m_wallet.getSeed(res.key))
        throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_UNKNOWN_ERROR, std::string("The wallet is non-deterministic. Cannot display seed."));
   if (0 == req.key_type.compare("paperwallet")) {
	AccountKeys keys;
	m_wallet.getAccountKeys(keys);
	res.key = Tools::Base58::encode_addr(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
		std::string(reinterpret_cast<char*>(&keys), sizeof(keys)));
   }
  return true;
}

bool wallet_rpc_server::on_gen_paymentid(const wallet_rpc::COMMAND_RPC_GEN_PAYMENT_ID::request& req,
	wallet_rpc::COMMAND_RPC_GEN_PAYMENT_ID::response& res) {
  std::string pid;
  try {
	pid = Common::podToHex(Crypto::rand<Crypto::Hash>());
  } catch (const std::exception& e) {
	throw JsonRpc::JsonRpcError(WALLET_RPC_ERROR_CODE_UNKNOWN_ERROR, std::string("Internal error: can't generate Payment ID: ") + e.what());
  }
  res.payment_id = pid;
  return true;
}

}
