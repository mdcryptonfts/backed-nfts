#pragma once

void backednfts::listen_for_burn(name asset_owner, uint64_t asset_id, name collection_name,
  			name schema_name, int32_t template_id, vector<asset> backed_tokens, atomicdata::ATTRIBUTE_MAP old_immutable_data,
  			atomicdata::ATTRIBUTE_MAP old_mutable_data, name asset_ram_payer) {

	const name tkcontract = get_first_receiver();

	if(tkcontract != ATOMICASSETS_CONTRACT){ return; }

	if(is_blacklisted(asset_owner)){ return; }

	auto c_it = config_t.require_find(0, ERR_CONFIG_NOT_FOUND);

	if(c_it->filter_type == 1){ //collections
		handle_type_1(asset_owner, collection_name);
	}

	else if(c_it->filter_type == 2){ //schemas
		handle_type_2(asset_owner, collection_name, schema_name);
	}

	else if(c_it->filter_type == 3){ //templates
		//nothing to do if template is -1
		if(template_id <= 0){
			return;
		}
		
		handle_type_3(asset_owner, uint64_t(template_id));;
	}		

}

void backednfts::handle_type_1(const name& user, const name& collection_name){ //collections
	//see if the collection is in the burnables table
	auto burn_it = burn_t.find(collection_name.value);

	//see if in mintables table
	auto mint_it = mint_t.find(collection_name.value);

	//if neither, nothing for us to do
	if(burn_it == burn_t.end() && mint_it == mint_t.end()){
		return;
	}

	//see if user exists for this collection yet
	users_table users_t = users_table(get_self(), user.value);
	auto user_it = users_t.find(collection_name.value);

	uint64_t current_burn_count = user_it == users_t.end() ? 1 : user_it->current_burn_count + 1;
	uint64_t all_time_burn_count = user_it == users_t.end() ? 1 : user_it->all_time_burn_count + 1;

	//process tokens if necessary
	if(burn_it != burn_t.end()){

		for(FUNGIBLE_TOKEN t : burn_it->reward_amounts){
			/**
			 *  amount should always be > 0, but this is an extra safety precaution just in case
			 */

			if(t.quantity.amount > 0){
				transfer_tokens(user, t.quantity, t.token_contract, std::string("your reward for burning an NFT from " + collection_name.to_string() + " collection"));
			}
		}

	}

	//process nfts if necessary
	if(mint_it != mint_t.end()){
		//only mint if user reached the threshold
		if(current_burn_count >= mint_it->burns_required){
			//reset the count
			current_burn_count = 0;

			uint8_t minted = 0;
			while(minted < mint_it->quantity_to_mint){
				mint(user, mint_it->reward_collection, mint_it->reward_schema, int32_t(mint_it->reward_template));

				minted += 1;
			}
		}

	}

	//upsert the user's info with the new counts
	if(user_it == users_t.end()){
		users_t.emplace(get_self(), [&](auto &_u){
			_u.ID = collection_name.value;
			_u.current_burn_count = current_burn_count;
			_u.all_time_burn_count = all_time_burn_count;
		});
	} else {
		users_t.modify(user_it, get_self(), [&](auto &_u){
			_u.current_burn_count = current_burn_count;
			_u.all_time_burn_count = all_time_burn_count;
		});
	}

}

void backednfts::handle_type_2(const name& user, const name& collection_name, const name& schema_name){ //schemas
	//see if the collection is in the burnables table
	const uint128_t col_sch_combo = mix64to128(collection_name.value, schema_name.value);
    auto burn_by_col_sch = burn_t.get_index<"colandschema"_n>();
	auto burn_it = burn_by_col_sch.find(col_sch_combo);

	//see if in mintables table
	auto mint_by_col_sch = mint_t.get_index<"colandschema"_n>();
	auto mint_it = mint_by_col_sch.find(col_sch_combo);

	//if neither, nothing for us to do
	if(burn_it == burn_by_col_sch.end() && mint_it == mint_by_col_sch.end()){
		return;
	}

	//see if user exists for this collection yet
	users_table users_t = users_table(get_self(), user.value);
	auto user_by_col_sch = users_t.get_index<"colandschema"_n>();
	auto user_it = user_by_col_sch.find(col_sch_combo);

	uint64_t current_burn_count = user_it == user_by_col_sch.end() ? 1 : user_it->current_burn_count + 1;
	uint64_t all_time_burn_count = user_it == user_by_col_sch.end() ? 1 : user_it->all_time_burn_count + 1;

	//process tokens if necessary
	if(burn_it != burn_by_col_sch.end()){

		for(FUNGIBLE_TOKEN t : burn_it->reward_amounts){
			/**
			 *  amount should always be > 0, but this is an extra safety precaution just in case
			 */

			if(t.quantity.amount > 0){
				transfer_tokens(user, t.quantity, t.token_contract, std::string("your reward for burning an NFT from " + schema_name.to_string() + " schema"));
			}
		}

	}

	//process nfts if necessary
	if(mint_it != mint_by_col_sch.end()){
		//only mint if user reached the threshold
		if(current_burn_count >= mint_it->burns_required){
			//reset the count
			current_burn_count = 0;

			uint8_t minted = 0;
			while(minted < mint_it->quantity_to_mint){
				mint(user, mint_it->reward_collection, mint_it->reward_schema, int32_t(mint_it->reward_template));

				minted += 1;
			}
		}

	}

	//upsert the user's info with the new counts
	if(user_it == user_by_col_sch.end()){
		users_t.emplace(get_self(), [&](auto &_u){
			_u.ID = users_t.available_primary_key();
			_u.col_sch_combo = col_sch_combo;
			_u.current_burn_count = current_burn_count;
			_u.all_time_burn_count = all_time_burn_count;
		});
	} else {
		user_by_col_sch.modify(user_it, get_self(), [&](auto &_u){
			_u.current_burn_count = current_burn_count;
			_u.all_time_burn_count = all_time_burn_count;
		});
	}

}

void backednfts::handle_type_3(const name& user, const uint64_t& template_id){ //templates
	//see if the collection is in the burnables table
	auto burn_it = burn_t.find(template_id);

	//see if in mintables table
	auto mint_it = mint_t.find(template_id);

	//if neither, nothing for us to do
	if(burn_it == burn_t.end() && mint_it == mint_t.end()){
		return;
	}

	//see if user exists for this collection yet
	users_table users_t = users_table(get_self(), user.value);
	auto user_it = users_t.find(template_id);

	uint64_t current_burn_count = user_it == users_t.end() ? 1 : user_it->current_burn_count + 1;
	uint64_t all_time_burn_count = user_it == users_t.end() ? 1 : user_it->all_time_burn_count + 1;

	//process tokens if necessary
	if(burn_it != burn_t.end()){

		for(FUNGIBLE_TOKEN t : burn_it->reward_amounts){
			/**
			 *  amount should always be > 0, but this is an extra safety precaution just in case
			 */

			if(t.quantity.amount > 0){
				transfer_tokens(user, t.quantity, t.token_contract, std::string("your reward for burning template " + to_string(template_id) + " collection"));
			}
		}

	}

	//process nfts if necessary
	if(mint_it != mint_t.end()){
		//only mint if user reached the threshold
		if(current_burn_count >= mint_it->burns_required){
			//reset the count
			current_burn_count = 0;

			uint8_t minted = 0;
			while(minted < mint_it->quantity_to_mint){
				mint(user, mint_it->reward_collection, mint_it->reward_schema, int32_t(mint_it->reward_template));

				minted += 1;
			}
		}

	}

	//upsert the user's info with the new counts
	if(user_it == users_t.end()){
		users_t.emplace(get_self(), [&](auto &_u){
			_u.ID = template_id;
			_u.current_burn_count = current_burn_count;
			_u.all_time_burn_count = all_time_burn_count;
		});
	} else {
		users_t.modify(user_it, get_self(), [&](auto &_u){
			_u.current_burn_count = current_burn_count;
			_u.all_time_burn_count = all_time_burn_count;
		});
	}

}