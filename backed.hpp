#pragma once

#define CONTRACT_NAME "backednfts"
#define mix64to128(a, b) (uint128_t(a) << 64 | uint128_t(b))

#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/symbol.hpp>
#include <string>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>
#include "include/atomic.hpp"
#include "include/atomicdata.hpp"
#include<map>
#include "structs.hpp"
#include "constants.hpp"
#include "tables.hpp"

using namespace eosio;


CONTRACT backednfts : public contract {
	public:
		using contract::contract;

		backednfts(name receiver, name code, datastream<const char *> ds):contract(receiver, code, ds){}

		//Main Actions
		ACTION addauthaccts(const std::vector<eosio::name>& accounts_to_add);
		ACTION addblacklist(const eosio::name& user, const std::vector<eosio::name>& wallets_to_blacklist);
		ACTION initconfig(const eosio::name& filter_type);
		ACTION removeburn(const eosio::name& user, const eosio::name& collection_name, const eosio::name& schema_name,
			const uint64_t& template_id);
		ACTION removemint(const eosio::name& user, const eosio::name& collection_name, const eosio::name& schema_name,
			const uint64_t& template_id);
		ACTION rmvauthacct(const name& account_to_remove);
		ACTION rmvblacklist(const eosio::name& user, const std::vector<eosio::name>& wallets_to_remove);
		ACTION upsertburn(const eosio::name& user, const eosio::name& collection_name, const eosio::name& schema_name,
			const uint64_t& template_id, const std::vector<FUNGIBLE_TOKEN>& reward_values);
		ACTION upsertmint(const eosio::name& user, const eosio::name& burn_collection, const eosio::name& burn_schema,
			const uint64_t& burn_template, const uint64_t& burns_required, const uint64_t& reward_template,
			const eosio::name& reward_collection, const uint8_t& quantity_to_mint);

		//Notifications
		[[eosio::on_notify("atomicassets::logburnasset")]] void listen_for_burn(name asset_owner, uint64_t asset_id, name collection_name,
  			name schema_name, int32_t template_id, vector<asset> backed_tokens, atomicdata::ATTRIBUTE_MAP old_immutable_data,
  			atomicdata::ATTRIBUTE_MAP old_mutable_data, name asset_ram_payer);

	private:

		//Tables
		black_table blacklist_t = black_table(get_self(), get_self().value);
		collections_t col_t = collections_t(ATOMICASSETS_CONTRACT, ATOMICASSETS_CONTRACT.value);
		burnable_tbl burn_t = burnable_tbl(get_self(), get_self().value);
		config_table config_t = config_table(get_self(), get_self().value);
		mint_table mint_t = mint_table(get_self(), get_self().value);

		//Functions
		void check_for_reward_token_duplicates(const std::vector<FUNGIBLE_TOKEN>& rewards);
		void check_token_exists(const symbol& token_symbol, const name& contract);
		void check_username(const name& user);
		void handle_type_1(const name& user, const name& collection_name);
		void handle_type_2(const name& user, const name& collection_name, const name& schema_name);
		void handle_type_3(const name& user, const uint64_t& template_id);
		bool is_authorized(const eosio::name& user, const eosio::name& collection);
		bool is_blacklisted(const eosio::name& user);
		void mint(const name& user, const name& collection, const name& schema, const int32_t& template_id);
		bool notified(const name& collection_name);
		bool schema_exists(const name& collection_name, const name& schema_name);
		bool template_exists(const name& collection_name, const uint64_t& template_id);
		void transfer_tokens(const name& user, const asset& amount_to_send, const name& contract, const std::string& memo);
		void validate_burns_required(const uint64_t& burns);
		void validate_quantity_to_mint(const uint8_t& qty);
};
