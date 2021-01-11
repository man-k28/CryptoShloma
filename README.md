## CryptoShloma

CryptoShloma is algo-trading bot for Binance exchange.

Crypto-trading algo (Hanukkah algo) based on set quantity of clever steps, when bot buys selected cryptocurrency to reduce first buy price. There are 2 tables with percent for all steps, 1st is percent of start price loss to activate step, 2nd is percent (which always lower, then 1st) of start price loss, when bot will buy crypto(it happens only when price is growing).

When crypto growing, bot takes profit. When falls, dont loosing money. :)

* [Article on Habr.com](https://habr.com/ru/sandbox/148944)
* [Article on VC.ru](https://vc.ru/u/689296-man-k28/195465-algo-treyding-bot-cryptoshloma)

## Compilation depends

* [Qt 5.12.10](https://www.qt.io/download-open-source) (min Qt 5.12.8 you can setup manually in cmake file). Not recommended version above (minor) 5.12 release due to bugs in the QWebSockets (you can catch SIGFAULT). <br>You only need to install Qt 5.12.10 Prebuilt Components for Desktop in Qt MaintenenceTool, all modules are not necessary.
* OpenSSL 1.1 (installed from Qt MaintenenceTool)
* [CuteLogger](https://github.com/man-k28/CuteLogger) [forked version]

## Platforms
* Windows 10 x64(x86)
* Debian 10 (Ubuntu 18,20)

## CMake options
```cmake
-DQt5_DIR="path to Qt cmake files" (required) - path to Qt framework
-DCuteLogget_STATIC=ON/OFF (optional, OFF default) - build library for static or shared linking
-DOPENSSL_CRYPTO_LIBRARY="path to file" (required) - path to libcrypto.so (libcrypto.dll for WIN)
-DOPENSSL_INCLUDE_DIR="path to dir" (required) - path to openssl headers
-DOPENSSL_SSL_LIBRARY="path to file" (optional) - path to libssl.so (libssl.dll for WIN)
```
## Binance **Spot** account
To connect your account, you need to generate a public and private key in your personal account and input the keys in the bot settings. **The bot only works for your spot account.**<br>
[How to create API keys for Binance](https://www.binance.com/en/support/articles/360002502072-How-to-create-API)

## Roadmap
- [x] Release Windows binary installer
- [ ] Release Linux deb installer

To be continued...

If you have a question, welcome to [Telegram](https://t.me/cryptoshloma)
