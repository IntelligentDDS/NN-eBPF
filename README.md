# 基于eBPF的神经网络实现
## 重新编译内核
编译内核版本到6.1.43，开启BTF。

拉取内核代码并解压
```shell
$ wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.43.tar.xz
$ tar -xvf linux-6.1.43.tar.xz
```

安装编译内核时需要的包
```shell
$ sudo apt install make build-essential libncurses5-dev bison flex libssl-dev dwarves libelf-dev
```

生成内核配置文件`.config`
```shell
$ cd linux-6.1.43
$ make mrproper && \
  make clean && \
  make menuconfig
```

编辑内核配置文件`.config`中的如下配置
```shell
CONFIG_BPF=y
CONFIG_BPF_SYSCALL=y
CONFIG_BPF_JIT=y
CONFIG_TRACEPOINTS=y
CONFIG_BPF_LSM=y
CONFIG_DEBUG_INFO=y
CONFIG_DEBUG_INFO_BTF=y
CONFIG_LSM="bpf"
CONFIG_SYSTEM_TRUSTED_KEYS=""
CONFIG_SYSTEM_REVOCATION_KEYS=""
```
编译内核
```shell
$ make -j$(nproc)
$ sudo make modules_install -j$(nproc)
$ sudo make install -j$(nproc)
$ sudo reboot now
```
## 重新编译libbpf和bpftool
在本项目文件夹下执行
```shell
$ make -j$(nproc)
```

## 训练模型并量化模型参数
进入`src`文件夹，训练模型，生成的`mlp.th`文件保存了模型参数
```shell
python3 mlp_train.py
```

量化模型参数，生成的`mlp_params.bpf.h`保存了量化结果
```shell
python3 mlp_quant.py ./mlp.th 16
```

训练所用的数据保存在`dataset`文件夹下

## 编译eBPF XDP代码
进入`src`文件夹执行
```shell
$ make
```
编译出`.output/xdp`和`.output/hot_updating`

## 运行、热更新参数
进入`src`文件夹，加载xdp程序
```shell
$ sudo .output/xdp
```
在xdp程序加载成功后，在另一个终端，并且在`src`文件夹下，以热更新的方式加载模型参数
```shell
$ sudo .output/hot_updating
```
查看实时输出
```shell
sudo cat /sys/kernel/debug/tracing/trace_pipe
```

## 可运行的实例
`dataset-instance`文件夹下存放了正常和入侵的实例，可以实时运行检测模型的实时分析效果