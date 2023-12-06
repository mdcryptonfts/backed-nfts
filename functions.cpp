#pragma once

void backednfts::check_for_reward_token_duplicates(const std::vector<FUNGIBLE_TOKEN>& rewards){
    std::set<std::pair<eosio::symbol, eosio::name>> unique_rewards;

    for (const auto& r : rewards) {
        auto pair = std::make_pair(r.quantity.symbol, r.token_contract);

        if(unique_rewards.find(pair) != unique_rewards.end()) {
            eosio::check(false, "Duplicate reward found");
        } else {
            unique_rewards.insert(pair);
        }
    }
}

void backednfts::check_token_exists(const symbol& token_symbol, const name& contract){
	const uint64_t raw_token_symbol = token_symbol.code().raw();
	const uint128_t symbol_contract_combo = mix64to128(raw_token_symbol, contract.value);
	stat_table stat(contract, token_symbol.code().raw());
	auto stat_itr = stat.find(token_symbol.code().raw());
	check(stat_itr != stat.end(), "That token does not exist on that contract");
	check(stat_itr->supply.symbol == token_symbol, "Symbol mismatch. You probably put the wrong amount of decimals in the precision field");
	return;
}

void backednfts::check_username(const name& user){
	if(user == get_self()){
		return;
	}
	auto itr = config_t.require_find(0, ERR_CONFIG_NOT_FOUND);
	check(std::find(itr->authorized_accounts.begin(), itr->authorized_accounts.end(), user) != itr->authorized_accounts.end(), "You aren't authorized to perform this action");	
	return;
}

bool backednfts::is_authorized(const eosio::name& user, const eosio::name& collection){
    auto col_itr = col_t.require_find(collection.value, (collection.to_string() + " collection does not exist").c_str());

    return std::find(col_itr->authorized_accounts.begin(), col_itr->authorized_accounts.end(), user) != col_itr->authorized_accounts.end();
}

bool backednfts::is_blacklisted(const eosio::name& user){
	auto it = blacklist_t.find(user.value);
	if(it != blacklist_t.end()){
		return true;
	}
	return false;
}

void backednfts::mint(const name& user, const name& collection, const name& schema, const int32_t& template_id){  
	action(
		permission_level(get_self(), "active"_n),
	 		ATOMICASSETS_CONTRACT,
	 		"mintasset"_n,
	 		std::make_tuple(
	 		get_self(), // authorized_minter
	 		collection, // collection_name
		schema, // schema_name
		template_id, // template_id
		user, // new_asset_owner
		std::map<std::string, atomicassets::ATOMIC_ATTRIBUTE>(), // immutable_data
		std::map<std::string, atomicassets::ATOMIC_ATTRIBUTE>(), // mutable_data
		std::vector<asset>() // tokens_to_back
		)
	).send();
}

bool backednfts::notified(const name& collection_name){
	auto col_itr = col_t.require_find(collection_name.value, "Collection does not exist");

	if(std::find(col_itr->notify_accounts.begin(), col_itr->notify_accounts.end(), get_self()) == col_itr->notify_accounts.end()){
		return false;
	}
	else{
		return true;
	}
}

bool backednfts::schema_exists(const name& collection_name, const name& schema_name){
	atomicassets::schemas_t schematable = atomicassets::schemas_t(ATOMICASSETS_CONTRACT, collection_name.value);
	auto itr = schematable.find(schema_name.value);
	if(itr != schematable.end()){
		return true;
	}
	return false;
}

bool backednfts::template_exists(const name& collection_name, const uint64_t& template_id){
	atomicassets::templates_t temptbl(ATOMICASSETS_CONTRACT, collection_name.value);
	auto temp_itr = temptbl.find(template_id);
	if(temp_itr != temptbl.end()){
		return true;
	}
	return false;
}

void backednfts::transfer_tokens(const name& user, const asset& amount_to_send, const name& contract, const std::string& memo){
    action(permission_level{get_self(), "active"_n}, contract,"transfer"_n,std::tuple{ get_self(), user, amount_to_send, memo}).send();
    return;
}

void backednfts::validate_burns_required(const uint64_t& burns){
	if(burns == 0){
		check(false, "burns_required must be positive");
	}
	else if(burns > 10000){
		check(false, "burns_required must be 10k or less");
	}
	return;
}

void backednfts::validate_quantity_to_mint(const uint8_t& qty){
	if(qty == 0){
		check(false, "quantity_to_mint must be positive");
	}
	else if(qty > MAX_QUANTITY_TO_MINT){
		check(false, ("quantity_to_mint must be " + (to_string(MAX_QUANTITY_TO_MINT)) + " or less").c_str());
	}
	return;
}