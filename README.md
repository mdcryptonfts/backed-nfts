## Bridge Contract Documentation

### This contract is still under construction, we will make an announcement in our [Telegram](https://t.me/hoodpunks) when it's launched

### What Is This?

This contract allows an NFT collection owner to "back" their collection's NFTs with any WAX token(s) (and NFTs) that they like. You can specify **ONE** of the following:

- Reward every NFT in one or more **collections**
- Reward every NFT in one or more **schemas** (even if they are in different collections)
- Reward every NFT with a specific **template ID** (even if the templates are in different collections/schemas)

### Important Notes

- Whatever WAX address you upload this contract to must be added to your NFT collection(s) as a **notify account** by going to **AtomicHub > Manage Collection** and then editing the notify accounts.

- This contract **will not work** unless you add it as a notify account

- Because of the above limitations, **users who buy NFTs that are "backed" by this contract should understand that the collection owner can remove the backing at any time by disabling collection notifications, or by not having enough tokens in the contract to cover rewards**

- Essentially, this contract is a tool for collection owners to reward their holders when an NFT is burned. It should **NOT** be treated as any sort of "guarantee", so if you are an NFT holder, then make sure you trust the collection owner who is utilizing this contract... no guarantees can be provided by the contract author.

- This contract must be an **AUTHORIZED** account on any collections that you plan to **MINT** reward NFTs from. You can not mint NFTs from a collection unless you are authorized on it.

- You must add the **eosio.code** permission to the account that you deploy the contract to, or else the contract will not be able to send reward tokens to users.

- **RAM:** you need <contract_size> bytes of RAM to deploy the contract initially. Then you also need extra ram for storing user data every time they burn an eligible NFT. The amounts can vary depending on the configuration, and whether or not a new record needs to be created. The first time a user burns an NFT that meets a specific criteria, a new record is created - which costs RAM. After that, any other burns for that user (if the NFT meets the same criteria) will update the existing record, which won't cost any more RAM. You also need RAM for minting NFTs, if you choose to reward a user with NFTs. You also *may* need RAM if you send a particular fungible token to a user who has never held that token before. 

- If you don't have tokens in the contract to cover the burn rewards, transactions will fail when users try to burn an eligible NFT.

- If a reward NFT template has reached its max supply, transactions will fail when users try to burn an eligible NFT (provided they are eligible for a reward NFT on that particular transaction)

- Using this contract adds extra computational logic when users burn NFTs from collections that notify this contract. This means it will cost users more CPU than a usual burn. The amount of CPU depends on how complex the transaction gets - the more tokens/NFTs you reward a user with, the more expensive. Transferring 3 different tokens is more expensive than doing 1 token transfer. Minting 50 NFTs is 50x more expensive than minting 1 NFT. Rewarding with tokens and ALSO rewarding with NFTs is more expensive than ONLY rewarding with tokens, or ONLY rewarding with NFTs.

- YOU as the contract owner, only need CPU for deploying the contract and interacting with it. The way WAX blockchain works, means that users who burn NFTs will be the ones paying the CPU costs of minting their NFTs, transferring reward tokens to them etc.


### Features

- Reward with fungible tokens when an NFT is burned
- Reward users with an NFT when they reach a certain amount of burns
- Blacklist users