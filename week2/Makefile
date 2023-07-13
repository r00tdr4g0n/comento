# for buildroot
BUILDROOT_DOWNLOAD:=https://buildroot.org/downloads/buildroot-2023.02.1.tar.gz
BUILDROOT_ORIGIN_DIR:=buildroot-2023.02.1
BUILDROOT_MOD_DIR:=buildroot
BUILDROOT_FILE_NAME:=$(BUILDROOT_ORIGIN_DIR).tar.gz
BUILDROOT_FULL_DIR:=`pwd`/$(BUILDROOT_MOD_DIR)
BUILDROOT_CONFIG:=$(BUILDROOT_FULL_DIR)/board/qemu/aarch64-virt/linux.config
BUILDROOT_MENUCONFIG=`pwd`/.buildroot_menuconfig

# for kernel
KERNEL_DOWNLOAD:=https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.2.5.tar.xz
KERNEL_ORIGIN_DIR:=linux-6.2.5
KERNEL_FILE_NAME:=$(KERNEL_ORIGIN_DIR).tar.xz
KERNEL_MOD_DIR:=linux
KERNEL_DIR:=kernel
KERNEL_CONFIG_DEST:=`pwd`/$(KERNEL_DIR)/$(KERNEL_MOD_DIR)/arch/arm64/configs/qemu_defconfig
KERNEL_FULL_DIR:=`pwd`/$(KERNEL_DIR)/$(KERNEL_MOD_DIR)
KERNEL_BUILD_ARCH:=arm64

# for toolchain
TOOLCHAIN_DOWNLOAD:=https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz
TOOLCHAIN_ORIGIN_DIR:=gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu
TOOLCHAIN_FILE_NAME:=$(TOOLCHAIN_ORIGIN_DIR).tar.xz
TOOLCHAIN_DIR:=toolchain
TOOLCHAIN_FULL_DIR:=`pwd`/$(TOOLCHAIN_DIR)

KERNEL_BUILD_TOOLCHAIN:=`pwd`/../../$(TOOLCHAIN_DIR)

CORE_CNT:=`cat /proc/cpuinfo | grep processor | wc -l`
CURR_DIR:=`pwd`

# all
all: buildroot-download buildroot-config buildroot-build kernel-download kernel-config kernel-build

buildroot-check:
	@if [ -e $(BUILDROOT_MOD_DIR) ] && [ -d $(BUILDROOT_MOD_DIR) ]; then\
		echo "\033[31;47;1m(!) already exist $(BUILDROOT_MOD_DIR)\033[0m";\
		exit 1;\
	fi

kernel-check:
	@if [ -e $(KERNEL_DIR) ] && [ -d $(KERNEL_DIR) ]; then\
		echo "\033[31;47;1m(!) already exist $(BUILDROOT_MOD_DIR)\033[0m";\
		exit 1;\
	fi

# if not exist buildroot directory, download buildroot
buildroot-download: buildroot-check
	@wget $(BUILDROOT_DOWNLOAD)
	@tar xvf $(BUILDROOT_FILE_NAME)
	@mv $(BUILDROOT_ORIGIN_DIR) $(BUILDROOT_MOD_DIR)
	@rm $(BUILDROOT_FILE_NAME)

# set defconfig and run menuconfig
buildroot-config:
	@cd $(BUILDROOT_MOD_DIR);make qemu_aarch64_virt_defconfig
	@sudo apt install libncurses-dev
	@cp $(BUILDROOT_MENUCONFIG) $(BUILDROOT_MOD_DIR)/.config

# build buildroot
buildroot-build: 
	@cd $(BUILDROOT_FULL_DIR);make -j$(CORE_CNT)

# install toolchain
toolchain-download:
	@wget $(TOOLCHAIN_DOWNLOAD)
	@tar xvf $(TOOLCHAIN_FILE_NAME)
	@mv $(TOOLCHAIN_ORIGIN_DIR) $(TOOLCHAIN_DIR)
	@rm -r $(TOOLCHAIN_FILE_NAME)

# if not exist kernel directory, download kernel
kernel-download: kernel-check toolchain-download
	@mkdir $(KERNEL_DIR)
	@cd $(KERNEL_DIR);wget $(KERNEL_DOWNLOAD)
	@cd $(KERNEL_DIR);tar xvf $(KERNEL_FILE_NAME)
	@cd $(KERNEL_DIR);mv $(KERNEL_ORIGIN_DIR) $(KERNEL_MOD_DIR)
	@cd $(KERNEL_DIR);rm -r $(KERNEL_FILE_NAME)
	@sudo apt install libelf-dev libssl-dev flex bison

# set defconfig
kernel-config:
	@cp $(BUILDROOT_CONFIG) $(KERNEL_CONFIG_DEST)
	@cd $(KERNEL_DIR)/$(KERNEL_MOD_DIR);ARCH=arm64 make qemu_defconfig

# build kernel
kernel-build:
	@cd $(KERNEL_DIR)/$(KERNEL_MOD_DIR);ARCH=arm64 CROSS_COMPILE=$(KERNEL_BUILD_TOOLCHAIN)/bin/aarch64-none-linux-gnu- make -j$(CORE_CNT)

# run linux by qemu
run:
	@sudo apt install qemu-system-arm
	@qemu-system-aarch64 \
		-kernel $(KERNEL_FULL_DIR)/arch/arm64/boot/Image \
		-drive format=raw,file=$(BUILDROOT_FULL_DIR)/output/images/rootfs.ext4,if=virtio \
		-append "root=/dev/vda console=ttyAMA0 nokaslr" \
		-nographic -M virt -cpu cortex-a72 \
		-m 2G \
		-smp 2

.PHONY:
clean:
	@rm -rf $(BUILDROOT_MOD_DIR)
	@rm -rf $(KERNEL_DIR)
	@rm -rf $(TOOLCHAIN_DIR)
