# LLVM CFG

将 LLVM IR 的控制流图导出为 Markdown Mermaid。

```bash
./llvmcfg testfile.bc > testfile.md
```

编译本工具命令如下，需要将 `LLVM_DIR` 替换成自己的 LLVM 所在目录。

```bash
mkdir -p build && cd build
cmake -DLLVM_DIR=${LLVM_DIR} -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
cd ..
cp build/llvmcfg .
```

如果是 make install 的 LLVM，则填入 install prefix，例如 `/usr/local/llvm-10` ，该目录的结构如下：

```
bin/  include/  lib/  libexec/  share/
```

如果只是从源码 cmake 编译的，则填入 llvm project 目录下的 `/build/lib/cmake/llvm` 子目录，例如 `../llvm-project-10.0.0.src/build/lib/cmake/llvm` 。