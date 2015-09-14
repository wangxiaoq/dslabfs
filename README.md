# dslabfs
This is a simple filesystem named after my labratory. Use the new kernfs interface, just as an example to learn the kernel.

Usage:
	1.	Please patch the dslabfs.4.2.patch to linux 4.2 source,
		then compile the kernel, and start the new kernel.

	2.	Type `make' to compile the module.

	3.	Use the following command to install the module:
			make install
		this may require you to type your password.

	4.	mount the new file system:
			mount -t dslabfs none /mnt
		you will find that under the /mnt is a file 'dslab',
		it can be written, read. Also you can create directory
		and rename it or remove it.

	5.	To umount the file system, use the following command:
			umount /mnt

	6.	To uninstall the file system, use command:
			make uninstall
		this may require you to type your password.

Welcome to visit our laboratory websites:
	http://dslab.lzu.edu.cn/
