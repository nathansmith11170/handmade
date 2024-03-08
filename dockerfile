# Start from the latest Ubuntu LTS image
FROM ubuntu:22.04

# Install necessary packages
RUN apt-get update && \
    apt-get install -y apt-utils perl-base libterm-readline-perl-perl gnupg wget && \
    apt-get install -y software-properties-common lsb-release && \
    apt-get install -y git curl build-essential && \
    apt-get clean && \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null && \
    apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" && \
    apt-get update && \
    apt-get install -y cmake && \
    rm -rf /var/lib/apt/lists/* && \
    cd ~ && \
    git clone --filter=blob:none https://github.com/llvm/llvm-project.git && \
    cd llvm-project && \
    git checkout 461274b && \
    cmake -S llvm -B build -G Ninja -DLLVM_ENABLE_PROJECTS='clang;clang-tools-extra;lldb;lld' -DLLVM_PARALLEL_LINK_JOBS=4 -DLLVM_PARALLEL_COMPILE_JOBS=8 -DCMAKE_BUILD_TYPE=Release && \
    ninja -C build check-llvm install && \
    sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 40 && \
    sudo update-alternatives --install /usr/bin/c++ c++ /usr/local/bin/clang++ 60 && \
    echo "\n" | sudo update-alternatives --config c++ && \
    sudo update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 40 && \
    sudo update-alternatives --install /usr/bin/cc cc /usr/local/bin/clang 60 && \
    echo "\n" | sudo update-alternatives --config cc && \
    sudo update-alternatives --install /usr/bin/ld ld /usr/bin/x86_64-linux-gnu-ld 40 && \
    sudo update-alternatives --install /usr/bin/ld ld /usr/local/bin/ld64.lld 60 && \
    echo "\n" | sudo update-alternatives --config ld

