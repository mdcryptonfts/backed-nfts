## Step By Step Instructions

### Step 1 - RAM

Buy 1.4M bytes of RAM for the account that you plan on deploying the contract to

### Step 2 - Deploy Contract

If you'd like to avoid downloading any software, and avoid having to learn how to become a programmer, you can do this through [our website](https://waxdao.io) by buying/burning a ~$5 NFT and clicking a few buttons.

This also helps support the contract creator so that we can continue building cool tools like this for you, so it's a win/win.

If you **don't** want to go that route, then you likely don't need any instructions to do it on your own, so go right ahead - the code is open-source for a reason.

---

## ATTENTION

From this point forward, it is probably much easier for you if you use the [interface that we built for you](https://waxdao.io/backed-nfts) for the rest of the steps.

We'd still recommend reading the rest of this document though, as it will help you understand how the contract works.

If you're a more technical user and would like to do this all on your own, without our UI, continue reading.

---

### Step 3 - initconfig

You need to choose how you want to set up the reward system. No matter which of the following 3 options you choose, you'll be able to reward users with NFTs and/or with fungible tokens when they burn NFTs.

The only thing that you have to choose, is what will they burn in order to receive rewards? Don't worry, you aren't choosing the actual assets yet - BUT, you are choosing the **type** of assets, and you **can not change this once it's been configured**, so choose wisely.

Your 3 options are - **collections, schemas**, or **templates**

If you choose **collections** - every NFT in whatever collections you choose will be eligible for rewards.

If you choose **schemas**, then only the schemas you choose will be eligible.

If you choose **templates**, then only the template IDs you choose will be eligible.

Using the **initconfig** action on the contract, enter either **collections / schemas / templates** depending on your preference.

*Note: the config table on the contract will store this as a uint8_t for those of you who are technical users. 1 = collections, 2 = schemas, 3 = templates*

### Step 4 - eosio.code permission


### Step 5 - Add burnable assets

Now is when you're ready to start choosing which assets you want people to burn. There are 2 separate actions for this:

- **upsertburn** *for rewarding people with fungible tokens when they burn an NFT*

- **upsertmint** *for rewarding people by minting NFTs into their wallet*

You can also do both of these for a single asset if you like - meaning you can reward someone with tokens **and** NFTs for a single burn.

Let's discuss how these actions work. We will start off with upsertburn.

**upsertburn parameters**

- **user** *the account calling this action. Can be the contract, or an authorized wallet that you added*

- **collection_name** *which collection does the NFT belong to that you want people to burn?*

- **schema_name** *if you chose schemas as the config type, then this is required - otherwise you can leave it blank or put something random to get the transaction to work*

- **template_id** *if you chose templates as the config, then this is the template you want people to burn. If you chose another config, either leave this blank or put a random number if you're having issues submitting the transaction*

- **reward_values** *which tokens will you be rewarding people with, and how much of each will they get?*

reward_values takes a vector of FUNGIBLE_TOKEN (the struct can be found in the contract).

Here's an example of how you would add 2 tokens:

[{"quantity":"1.00000000 WAX", "token_contract":"eosio.token"}, {"quantity":"1.0000 SKATE", "token_contract":"skatecoinwax"}]