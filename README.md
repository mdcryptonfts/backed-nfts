## Bridge Contract Documentation

### This contract is still under construction, we will make an announcement in our [Telegram](https://t.me/hoodpunks) when it's launched

### What Is This?

This contract allows an NFT collection owner to "back" their collection's NFTs with any WAX token(s) (and NFTs) that they like. You can specify **ONE** of the following:

- Reward every NFT in one or more **collections**
- Reward every NFT in one or more **schemas** (even if they are in different collections)
- Reward every NFT with a specific **template ID** (even if the templates are in different collections/schemas)

### A Couple Of Important Notes

- Whatever WAX address you upload this contract to must be added to your NFT collection(s) as a **notify account** by going to **AtomicHub > Manage Collection** and then editing the notify accounts.

- This contract **will not work** unless you add it as a notify account

- Because of the above limitations, **users who buy NFTs that are "backed" by this contract should understand that the collection owner can remove the backing at any time by disabling collection notifications, or by not having enough tokens in the contract to cover rewards**

- Essentially, this contract is a tool for collection owners to reward their holders when an NFT is burned. It should **NOT** be treated as any sort of "guarantee", so if you are an NFT holder, then make sure you trust the collection owner who is utilizing this contract... no guarantees can be provided by the contract author.



### Features

- Reward with fungible tokens when an NFT is burned
- Reward users with an NFT when they reach a certain amount of burns
- Blacklist users