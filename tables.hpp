#pragma once

//Tables on other contracts

struct [[eosio::table]] collections {
	name collection_name;
	name author;
	bool allow_notify;
	vector<name> authorized_accounts;
	vector<name> notify_accounts;
	double market_fee;
	std::vector<uint8_t> serialized_data;
	uint64_t primary_key() const { return collection_name.value; }
};
typedef multi_index<"collections"_n, collections> collections_t;

struct [[eosio::table]] schemas {
	name schema_name;
	vector<atomicdata::FORMAT> format;
	uint64_t primary_key() const { return schema_name.value; }
};
typedef multi_index<"schemas"_n, schemas> atomics_s;

struct [[eosio::table]] stat {
  asset    	supply;
  asset 	max_supply;
  name 		issuer;

  uint64_t primary_key()const { return supply.symbol.code().raw(); }
};
typedef eosio::multi_index< "stat"_n, stat > stat_table;

struct [[eosio::table]] templates {
	int32_t template_id;
	name schema_name;
	bool transferable;
	bool burnable;
	uint32_t max_supply;
	uint32_t issued_supply;
	vector<uint8_t> immutable_serialized_data;
	int32_t primary_key() const { return template_id; }
};
typedef multi_index<"templates"_n, templates> atomic_temps;

//Tables on this contract

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] blacklist {
	eosio::name  			username;

	uint64_t primary_key() const { return username.value; }
};
using black_table = eosio::multi_index<"blacklist"_n, blacklist
>;

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] burnables {
	uint64_t  										ID;
	eosio::name 									collection_name;
	eosio::name  									schema_name;
	std::vector<FUNGIBLE_TOKEN>  	reward_amounts;

	uint64_t primary_key() const { return ID; }
	uint128_t secondarykey() const { return mix64to128(collection_name.value, schema_name.value); }
};
using burnable_tbl = eosio::multi_index<"burnables"_n, burnables,
eosio::indexed_by<"colandschema"_n, eosio::const_mem_fun<burnables, uint128_t, &burnables::secondarykey>>
>;

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] burnsbyuser { 
	//scoped by user

	uint64_t 		ID;
	uint128_t  	col_sch_combo;
	uint64_t 		current_burn_count;
	uint64_t 		all_time_burn_count;

	uint64_t primary_key() const { return ID; }
	uint128_t secondarykey() const { return col_sch_combo; }
};
using users_table = eosio::multi_index<"burnsbyuser"_n, burnsbyuser,
eosio::indexed_by<"colandschema"_n, eosio::const_mem_fun<burnsbyuser, uint128_t, &burnsbyuser::secondarykey>>
>;

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] config {
	uint64_t  								ID;
	uint8_t  									filter_type;
	std::vector<eosio::name> 	authorized_accounts;

	uint64_t primary_key() const { return ID; }
};
using config_table = eosio::multi_index<"config"_n, config
>;

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] nfts_to_mint {
	uint64_t  			ID;
	eosio::name   	burn_collection;
	eosio::name  		burn_schema;
	uint64_t 				burns_required;
	uint64_t	 			reward_template;
	eosio::name  		reward_collection;
	eosio::name  		reward_schema;
	uint8_t					quantity_to_mint;

	uint64_t primary_key() const { return ID; }
	uint128_t secondarykey() const { return mix64to128(burn_collection.value, burn_schema.value); }
};
using mint_table = eosio::multi_index<"nftstomint"_n, nfts_to_mint,
eosio::indexed_by<"colandschema"_n, eosio::const_mem_fun<nfts_to_mint, uint128_t, &nfts_to_mint::secondarykey>>
>;
