#!/bin/bash
#Script outline to install and build kernel.
#Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
    OUTDIR="/tmp/aeld"
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output" 
fi


mkdir -p "$OUTDIR"


if [ ! -d $OUTDIR ]; then

    echo -e " Error creating path"
    exit 1
fi


cd "$OUTDIR"


if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
cd "$OUTDIR"
cp -a ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image Image

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir ${OUTDIR}/rootfs

if [ ! -d "${OUTDIR}/rootfs" ]; then

    echo -e " Error creating path"
    exit 1
fi

cd ${OUTDIR}/rootfs
mkdir bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir usr/bin usr/lib usr/sbin
mkdir var/log

# TODO: Make and install busybox
cd "$OUTDIR"
sudo rm  -rf ${OUTDIR}/busybox
echo "Cloning busybox"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
    make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
else
    cd busybox
    # TODO:  Configure busybox
    make distclean
    make defconfig
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
    make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
fi


cd ${OUTDIR}/rootfs

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
echo "Adding Libraries"
cd ${OUTDIR}/rootfs
cp -a ${FINDER_APP_DIR}/libs/ld-linux-aarch64.so.1 lib
cp -a ${FINDER_APP_DIR}/libs/libm.so.6 lib64
cp -a ${FINDER_APP_DIR}/libs/libresolv.so.2 lib64
cp -a ${FINDER_APP_DIR}/libs/libc.so.6 lib64

# TODO: Make device nodes
echo "making Nodes"
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1

# TODO: Clean and build the writer utility
echo "Compiling Writer"
cd "$FINDER_APP_DIR"
make clean
make CROSS_COMPILE=${CROSS_COMPILE}

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
echo "Copying utilities"

$FINDER_APP_DIR
mkdir ${OUTDIR}/rootfs/home/conf
cp -a writer ${OUTDIR}/rootfs/home
cp -a autorun-qemu.sh ${OUTDIR}/rootfs/home
cp -a finder.sh ${OUTDIR}/rootfs/home
cp -a finder-test.sh ${OUTDIR}/rootfs/home
cp -a ../conf/username.txt ${OUTDIR}/rootfs/home/conf
cp -a ../conf/assignment.txt ${OUTDIR}/rootfs/home/conf


# TODO: Chown the root directory
echo "Chown directory"

cd ${OUTDIR}/rootfs
sudo chown -R root:root *

# TODO: Create initramfs.cpio.gz
echo "Create initram"
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUTDIR}
gzip initramfs.cpio


