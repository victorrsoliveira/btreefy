# Zephyr application example
---

## Install Zephyr SDK

Make sure you have Zephyr SDK properly installed. For more details on this, see [this link](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html).

## Initialize workspace

```bash
west init -l behavior-tree-app
```

## Compile

```shell
cd behavior-tree-app
west build -p -b nucleo_f091rc app
```