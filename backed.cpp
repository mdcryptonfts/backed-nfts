#pragma once

#include "backed.hpp"
#include "functions.cpp"
#include "on_notify.cpp"

ACTION backednfts::addauthaccts(const std::vector<eosio::name>& accounts_to_add)
{
	require_auth(get_self());

	auto it = config_t.require_find(0, ERR_CONFIG_NOT_FOUND);

	std::vector<eosio::name> existingAccounts = it->authorized_accounts;

	for(name a : accounts_to_add){
		if(!is_account(a)){
			check(false, (a.to_string() + " does not exist").c_str());
		}

		if(std::find(existingAccounts.begin(), existingAccounts.end(), a) != existingAccounts.end()){
			check(false, (a.to_string() + " is already an authorized account").c_str());
		}	

		existingAccounts.push_back(a);
	}

	config_t.modify(it, get_self(), [&](auto &_config){
		_config.authorized_accounts = existingAccounts;
	});		

}

ACTION backednfts::addblacklist(const eosio::name& user, const std::vector<eosio::name>& wallets_to_blacklist){
	require_auth(user);
	check_username(user);

	if(wallets_to_blacklist.size() == 0){
		check(false, "must add at least 1 wallet to blacklist");
	}

	for(eosio::name w : wallets_to_blacklist){
		if(!is_account(w)){
			check(false, (w.to_string() + " is not an account").c_str());
		}

		auto it = blacklist_t.find(w.value);
		if(it == blacklist_t.end()){
			blacklist_t.emplace(get_self(), [&](auto &_b){
				_b.username = w;
			});
		}

	}
}

ACTION backednfts::initconfig(const eosio::name& filter_type){
	require_auth(get_self());
	uint8_t filter_number;

	/**
	 *  used a "name" here instead of a uint8_t for semantic clarity, 
	 *  since a number would just confuse the average user and result 
	 *  in mistakes being made
	 */

	if(filter_type == "collections"_n){
		filter_number = 1;
	} else if(filter_type == "schemas"_n){
		filter_number = 2;
	} else if(filter_type == "templates"_n){
		filter_number = 3;
	} else {
		check(false, "expected collections, schemas or templates for filter_type");
	}

	auto it = config_t.find(0);

	if(it != config_t.end()){
		check(false, "config has already been initialized");
	}

	config_t.emplace(get_self(), [&](auto &_config){
		_config.ID = 0;
		_config.filter_type = filter_number;
	});
}

ACTION backednfts::removeburn(const eosio::name& user, const eosio::name& collection_name, const eosio::name& schema_name,
	const uint64_t& template_id)
{
	require_auth(user);
	check_username(user);

	uint8_t filter_type = config_t.require_find(0, ERR_CONFIG_NOT_FOUND)->filter_type;

	if(filter_type == 1){ //collections
		auto it = burn_t.require_find(collection_name.value, "this collection is not in table");
		it = burn_t.erase(it);
	}

	else if(filter_type == 2){ //schemas		
		const uint128_t col_sch_combo = mix64to128(collection_name.value, schema_name.value);
	    auto by_col_sch = burn_t.get_index<"colandschema"_n>();

	    auto it = by_col_sch.require_find(col_sch_combo, "this schema is not in the table");
	    it = by_col_sch.erase(it);
	}

	else if(filter_type == 3){ //templates
		auto it = burn_t.require_find(template_id, "this template is not in the table");
		it = burn_t.erase(it);
	}
}

ACTION backednfts::removemint(const eosio::name& user, const eosio::name& collection_name, const eosio::name& schema_name,
	const uint64_t& template_id)
{
	require_auth(user);
	check_username(user);

	uint8_t filter_type = config_t.require_find(0, ERR_CONFIG_NOT_FOUND)->filter_type;

	if(filter_type == 1){ //collections
		auto it = mint_t.require_find(collection_name.value, "this collection is not in table");
		it = mint_t.erase(it);
	}

	else if(filter_type == 2){ //schemas		
		const uint128_t col_sch_combo = mix64to128(collection_name.value, schema_name.value);
	    auto by_col_sch = mint_t.get_index<"colandschema"_n>();

	    auto it = by_col_sch.require_find(col_sch_combo, "this schema is not in the table");
	    it = by_col_sch.erase(it);
	}

	else if(filter_type == 3){ //templates
		auto it = mint_t.require_find(template_id, "this template is not in the table");
		it = mint_t.erase(it);
	}
}

ACTION backednfts::rmvauthacct(const name& account_to_remove)
{
	require_auth(get_self());

	auto it = config_t.require_find(0, ERR_CONFIG_NOT_FOUND);

	std::vector<eosio::name> existingAccounts = it->authorized_accounts;

	existingAccounts.erase(std::remove(existingAccounts.begin(), existingAccounts.end(), account_to_remove), existingAccounts.end());

	config_t.modify(it, get_self(), [&](auto &_config){
		_config.authorized_accounts = existingAccounts;
	});

}

ACTION backednfts::rmvblacklist(const eosio::name& user, const std::vector<eosio::name>& wallets_to_remove){
	require_auth(user);
	check_username(user);

	if(wallets_to_remove.size() == 0){
		check(false, "must add at least 1 wallet to remove");
	}

	for(eosio::name w : wallets_to_remove){
		auto it = blacklist_t.require_find(w.value, (w.to_string() + " is not blacklisted").c_str());
		it = blacklist_t.erase(it);
	}
}

/**
 *  upsertburn
 * 	use this to add/edit an asset that can be burned for token rewards
 *  uint64_t was used for template_id since an ID of -1 should never be used anyway
 */

ACTION backednfts::upsertburn(const eosio::name& user, const eosio::name& collection_name, const eosio::name& schema_name,
	const uint64_t& template_id, const std::vector<FUNGIBLE_TOKEN>& reward_values)
{
	require_auth(user);
	check_username(user);

	if(!notified(collection_name)){
		check(false, (get_self().to_string() + " is not a notify account on " + collection_name.to_string() + " collection").c_str());
	}	

	uint8_t filter_type = config_t.require_find(0, ERR_CONFIG_NOT_FOUND)->filter_type;

	if(reward_values.size() == 0){
		check(false, "must add at least 1 reward value");
	}

	check_for_reward_token_duplicates(reward_values);

	for(auto v : reward_values){
		check_token_exists(v.quantity.symbol, v.token_contract);

		if(v.quantity.amount <= 0){
			check(false, "amount must be positive");
		}

		if(v.quantity.amount > MAX_ASSET_AMOUNT){
			check(false, "quantity is too large");
		}
	}

	if(filter_type == 1){ //collections
		auto it = burn_t.find(collection_name.value);

		if(it == burn_t.end()){
			burn_t.emplace(get_self(), [&](auto &_burn){
				_burn.ID = collection_name.value;
				_burn.collection_name = collection_name;
				_burn.reward_amounts = reward_values;
			});
		} else {
			burn_t.modify(it, get_self(), [&](auto &_burn){
				_burn.reward_amounts = reward_values;
			});
		}
	}

	else if(filter_type == 2){ //schemas		
		atomics_s schemas_t = atomics_s(ATOMICASSETS_CONTRACT, collection_name.value);
		auto sch_it = schemas_t.require_find(schema_name.value, (schema_name.to_string() + " schema doesn't exist in " + collection_name.to_string() + " collection").c_str());

		const uint128_t col_sch_combo = mix64to128(collection_name.value, schema_name.value);
	    auto by_col_sch = burn_t.get_index<"colandschema"_n>();

	    auto it = by_col_sch.find(col_sch_combo);

		if(it == by_col_sch.end()){
			burn_t.emplace(get_self(), [&](auto &_burn){
				_burn.ID = burn_t.available_primary_key();
				_burn.collection_name = collection_name;
				_burn.schema_name = schema_name;
				_burn.reward_amounts = reward_values;
			});
		} else {
			by_col_sch.modify(it, get_self(), [&](auto &_burn){
				_burn.reward_amounts = reward_values;
			});
		}
	}

	else if(filter_type == 3){ //templates
		atomic_temps temps_t = atomic_temps(ATOMICASSETS_CONTRACT, collection_name.value);
		auto temp_it = temps_t.require_find(template_id, ("template " + to_string(template_id) + " doesn't exist in " + collection_name.to_string() + " collection").c_str());

		auto it = burn_t.find(template_id);

		if(it == burn_t.end()){
			burn_t.emplace(get_self(), [&](auto &_burn){
				_burn.ID = template_id;
				_burn.collection_name = collection_name;
				_burn.schema_name = temp_it->schema_name;
				_burn.reward_amounts = reward_values;
			});
		} else {
			burn_t.modify(it, get_self(), [&](auto &_burn){
				_burn.reward_amounts = reward_values;
			});
		}	
	}
}

ACTION backednfts::upsertmint(const eosio::name& user, const eosio::name& burn_collection, const eosio::name& burn_schema,
	const uint64_t& burn_template, const uint64_t& burns_required, const uint64_t& reward_template,
	const eosio::name& reward_collection, const uint8_t& quantity_to_mint)
{
	require_auth(user);
	check_username(user);

	if(!notified(burn_collection)){
		check(false, (get_self().to_string() + " is not a notify account on " + burn_collection.to_string() + " collection").c_str());
	}		

	uint8_t filter_type = config_t.require_find(0, ERR_CONFIG_NOT_FOUND)->filter_type;

	if(!is_authorized(get_self(), reward_collection)){
		check(false, ("we can't mint NFTs until you add " + get_self().to_string() + " as an authorized account on " + reward_collection.to_string()).c_str());
	}

	validate_burns_required(burns_required);
	validate_quantity_to_mint(quantity_to_mint);

	atomicassets::templates_t temptbl(ATOMICASSETS_CONTRACT, reward_collection.value);
	auto col_it = temptbl.require_find(reward_template, ("template id " + to_string(reward_template) + " doesn't exist in " + reward_collection.to_string() + " collection").c_str());

	if(filter_type == 1){ //collections
		auto m_it = mint_t.find(burn_collection.value);

		if(m_it == mint_t.end()){
			mint_t.emplace(get_self(), [&](auto &_m){
				_m.ID = burn_collection.value;
				_m.burn_collection = burn_collection;
				_m.burns_required = burns_required;
				_m.reward_template = reward_template;
				_m.reward_collection = reward_collection;
				_m.reward_schema = col_it->schema_name;
				_m.quantity_to_mint = quantity_to_mint;
			});
		} else {
			mint_t.modify(m_it, get_self(), [&](auto &_m){
				_m.burns_required = burns_required;
				_m.reward_template = reward_template;
				_m.reward_collection = reward_collection;
				_m.reward_schema = col_it->schema_name;
				_m.quantity_to_mint = quantity_to_mint;
			});			
		}
	}

	else if(filter_type == 2){ //schemas
		
		if(!schema_exists(burn_collection, burn_schema)){
			check(false, (burn_schema.to_string() + " schema does not exist in " + burn_collection.to_string() + " collection").c_str());
		};

		const uint128_t col_sch_combo = mix64to128(burn_collection.value, burn_schema.value);
	    auto by_col_sch = mint_t.get_index<"colandschema"_n>();

	    auto m_it = by_col_sch.find(col_sch_combo);

		if(m_it == by_col_sch.end()){
			mint_t.emplace(get_self(), [&](auto &_m){
				_m.ID = mint_t.available_primary_key();
				_m.burn_collection = burn_collection;
				_m.burn_schema = burn_schema;
				_m.burns_required = burns_required;
				_m.reward_template = reward_template;
				_m.reward_collection = reward_collection;
				_m.quantity_to_mint = quantity_to_mint;
			});
		} else {
			by_col_sch.modify(m_it, get_self(), [&](auto &_m){
				_m.burns_required = burns_required;
				_m.reward_template = reward_template;
				_m.reward_collection = reward_collection;
				_m.quantity_to_mint = quantity_to_mint;
			});			
		}
	}

	else if(filter_type == 3){ //templates
		if(!template_exists(burn_collection, burn_template)){
			check(false, ("template id " + to_string(burn_template) + " doesn't exist in " + burn_collection.to_string() + " collection").c_str());
		}

		auto m_it = mint_t.find(burn_template);

		if(m_it == mint_t.end()){
			mint_t.emplace(get_self(), [&](auto &_m){
				_m.ID = burn_template;
				_m.burn_collection = burn_collection;
				_m.burns_required = burns_required;
				_m.reward_template = reward_template;
				_m.reward_collection = reward_collection;
				_m.quantity_to_mint = quantity_to_mint;
			});
		} else {
			mint_t.modify(m_it, get_self(), [&](auto &_m){
				_m.burns_required = burns_required;
				_m.reward_template = reward_template;
				_m.reward_collection = reward_collection;
				_m.quantity_to_mint = quantity_to_mint;
			});			
		}
	}		

}

