![Version](https://img.shields.io/badge/Version-1.0.0-green.svg)

Bitcoin Toolkit
===============

Bitcoin Toolkit is a collection of command line tools that allow you to generate and manipulate public and private keys, create bitcoin addresses, create vanity addresses, and interact with bitcoin nodes.

This initial release is focused mainly around public and private key manipulation, providing tools allowing the user to generate key pairs in a variety of interesting and useful ways. Users can also change the format of existing keys, such as turning on and off compression, printing its hex or decimal value, and more.

This release also contains a node tool that dumps version information from any bitcoin node in JSON format. This is good for getting a node's status, last block, user agent, and a handfull of other things. More node tools are planned for future releases.

Table of Contents
-----------------

1. [Quick Intro](#quick-intro)
   * [Private Keys](#private-keys)
   * [Public Keys](#public-keys)
   * [Vanity Addresses](#vanity-addresses)
   * [Bitcoin Nodes](#bitcoin-nodes)
2. [Download and Install](#download-and-install)
3. [Usage](#usage)
4. [License](#license)
5. [Tips](#tips)

Quick Intro
-----------

#### Private Keys

Create a new random private key:
```
$ btk privkey -n
L3qvz11nTqDLYqNXZuf5zMiuJsRcEEd5oN2Fa9vrD8rPLL1dPDCD
```

Create a new random uncompressed private key:
```
$ btk privkey -nU
5JcNMHpGEVTuCBuhrrAqZtQS3k4PWuSwXYrbNPMq9LEDQA79Pff
```

Create a private key from the SHA256 hash value of an ASCII string:
```
$ echo "this is my secret string" | btk privkey -s
L4Cp75ZzpF5AAKwN64VBTxhMfHa8bpTw6mt6uq5T5buaEEuVU1Sz
```

Create a private key from the SHA256 hash value of arbitrary binary data:
```
$ cat secret.dat | btk privkey -b
L33qZ2xKYegwnhyxhdeeCCcLU7cSbbG57aG5JrdXjAUqf762wZAy
```

Create a private key with a specific decimal value:
```
$ echo "101" | btk privkey -d
KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU7ufmjaJwj
```

Convert the previous command output to hexadecimal format:
```
$ echo "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU7ufmjaJwj" | btk privkey -H
0000000000000000000000000000000000000000000000000000000000000065
```

Create a new random private key for testnet:
```
$ btk privkey -nT
cPz6uKLTGD9y55GxsZcD57k7Pt2WGwHD5dePb6RsM9BkunzWcGaA
```

Convert an uncompressed private key to compressed:
```
$ echo "5JcNMHpGEVTuCBuhrrAqZtQS3k4PWuSwXYrbNPMq9LEDQA79Pff" | btk privkey -C
Kzj7EZDcEYdjNP9dM22RN9tRZ73xNzVVfqbthPgwVmsKipnfSosK
```

Convert a private key to decimal format:
```
$ echo "5JcNMHpGEVTuCBuhrrAqZtQS3k4PWuSwXYrbNPMq9LEDQA79Pff" | btk privkey -D
47327905310046739235222537463578511200578158695347137990773448104177492139470
```

#### Public Keys

Public keys can only be derived from a private key. Input for the pubkey command should be in the form of a private key or data from which a private key can be derived.

Print the public key in standard address format:
```
$ echo "L4zz3k4TS2Rg4vchjGx7XUhyUschidxmevFnvAL7z8Ru78XcDaHU" | btk pubkey
12UNVuALofDnkCB1rznUY7iCP3T5xeyJur
```

Print the public key in uncompressed standard address format:
```
$ echo "L4zz3k4TS2Rg4vchjGx7XUhyUschidxmevFnvAL7z8Ru78XcDaHU" | btk pubkey -U
1Ecpnmz2no5MuKp6hpkgcPSbA71bRij46y
```

Print the public key in bech32 address format:
```
$ echo "L4zz3k4TS2Rg4vchjGx7XUhyUschidxmevFnvAL7z8Ru78XcDaHU" | btk pubkey -B
bc1qzqj56aex707wa5lxd9t85mtudhvnaqjkn36sqw
```

Print the public key in hexadecimal format:
```
$ echo "L4zz3k4TS2Rg4vchjGx7XUhyUschidxmevFnvAL7z8Ru78XcDaHU" | btk pubkey -H
03d15c40b508c6b2d13778b16af87ca2c76275ae4f1adb5db65b842fad7fe660cc
```

Print a new public key in standard address format, and include the private key in the output:
```
$ btk privkey -n | btk pubkey -P
L5U7RpRwu3r9xkn5ko91vgf331EVD2C3Q3k67fTvC6qs1Vp2Ftmk 1EiuECymwtzup6fPURHqKzF1uPQXcrdaKP
```

Print a new public and private key for testnet:
```
$ btk privkey -n | btk pubkey -PT
cVpTXJ1eYEySGcRB1NXgq5CDRGjgWcJAKVXav5TAeLG3YjGG5hFK msZKBKEHNPVgj45bziw7UqjsBhumoPGTh8
```

#### Vanity Addresses

Create a vanity address, in standard address format, matching the string "btc", using -i for a case insensitive match:
```
$ echo "btc" | btk vanity -i
1BtcyASYTqCFWHKjDPhz716j6jwh4SxVBw            Estimated Seconds: 149 of 422
Vanity Address Found!
Private Key: KyY3GXxNQa4Ufi7N75tuCpNS9nTMym323isZCUyw1TLQZPXzDUNk
Address:     1BtcyASYTqCFWHKjDPhz716j6jwh4SxVBw
```

Create a vanity address, in bech32 address format, matching the string "pry":
```
$ echo "pry" | btk vanity -B
bc1qpry94dn0zz805c4rd4fjprc9rzp7cnd5chrmys    Estimated Seconds: 336 of 368
Vanity address found!
Private Key: KyqfYMuojzuHjRjH5D8XE6nYNtX9gJufNEJ6WJTSsR8Z5xNSrbkv
Address:     bc1qpry94dn0zz805c4rd4fjprc9rzp7cnd5chrmys
```

#### Bitcoin Nodes

Print the version message info from a bitcoin node:
```
$ btk node -h seed.bitcoin.sipa.be
{
  "version": 70015,
  "services": {
    "bit 1": "NODE_NETWORK",
    "bit 3": "NODE_BLOOM",
    "bit 4": "NODE_WITNESS"
  },
  "timestamp": 1540926783,
  "addr_recv_services": {
  },
  "addr_recv_ip_address": "00000000000000000000ffff349072c5",
  "addr_recv_port": 49387,
  "addr_trans_services": {
    "bit 1": "NODE_NETWORK",
    "bit 3": "NODE_BLOOM",
    "bit 4": "NODE_WITNESS"
  },
  "addr_trans_ip_address": "00000000000000000000000000000000",
  "addr_trans_port": 0,
  "nonce": 3024123468520040665,
  "user_agent": "/Satoshi:0.15.1/",
  "start_height": 548011,
  "relay": true
}
```


Download and Install
--------------------
The following instructions are for unix and linux systems. Windows 10
users can install this project from within the linux subsystem.

Be sure that the following dependencies are installed on your system.
If not you can install them from your package manager.

1. libgmp
2. libgcrypt
3. libleveldb

To install them on debian systems:
```
sudo apt-get install libgmp-dev
sudo apt-get install libgcrypt20-dev
sudo apt-get install libleveldb-dev
```

You will also need `git` along with basic build tools like `make` and
`gcc`. Be sure these are also installed on your system.

Next, follow these instructions:

#### Install:
```
$ git clone https://github.com/bartobri/bitcoin-toolkit.git
$ cd ./bitcoin-toolkit
$ make
$ sudo make install
```

#### Uninstall:

```
$ sudo make uninstall
```

Usage
-----

Usage: `btk <command> [OPTIONS]`

Bitcoin Toolkit installs an executable named **btk**. It handles commands and options much like git. The first argument is a command, and all following arguments are command options.

See `btk help` for a list of available commands.

See `btk help <command>` for a list of options and more info about a command.

License
-------

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License. See [LICENSE](LICENSE) for
more details.
