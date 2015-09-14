/*
 * dslabfs.c - a simple filesystem using kernfs interface.
 *
 * Copyright 2015, Wang Xiaoqiang <wang_xiaoq@126.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/seq_file.h>

#define FS_NAME		"dslabfs"
#define FS_MAGIC	0x64668735
#define FS_MODE	S_IRWXU | S_IRUGO | S_IXUGO
#define MAX_SIZE	100

static char content[MAX_SIZE] = "hello, dslab!\n";

static int dslab_mkdir(struct kernfs_node *parent_kn, 
						const char *name, umode_t mode)
{
	struct kernfs_node *kn;
	int ret = 0;

	kn = kernfs_create_dir(parent_kn, name, mode, NULL);
	if (IS_ERR(kn)) {
		ret = PTR_ERR(kn);
		return ret;
	}

	kernfs_activate(kn);
	
	return ret;
}

static int dslab_rmdir(struct kernfs_node *kn)
{
	kernfs_break_active_protection(kn);
	kernfs_remove(kn);
	kernfs_unbreak_active_protection(kn);

	return 0;
}

static int dslab_rename(struct kernfs_node *kn, 
					struct kernfs_node *new_parent,
					const char *new_name_str)
{
	int ret = 0;	

	if (kernfs_type(kn) != KERNFS_DIR)
		return -ENOTDIR;
	if (kn->parent != new_parent)
		return -EIO;

	kernfs_break_active_protection(new_parent);
	kernfs_break_active_protection(kn);
	ret = kernfs_rename(kn, new_parent, new_name_str);
	kernfs_unbreak_active_protection(kn);
	kernfs_unbreak_active_protection(new_parent);

	return ret;
}

static struct kernfs_syscall_ops dslab_kf_syscall_ops = {
	.mkdir	=	dslab_mkdir,
	.rmdir	=	dslab_rmdir, 
	.rename	=	dslab_rename,
};

static ssize_t dslab_file_write(struct kernfs_open_file *of, char *buf, 
								size_t nbytes, loff_t off)
{
//	int ret;
	
//	ret = copy_from_user(content, buf, nbytes);
	
	strncpy(content, buf, nbytes);
	content[nbytes] = '\0';

//	return ret ?: nbytes;
}

static int dslab_seqfile_show(struct seq_file *m, void *arg)
{
	seq_printf(m, "%s", content);

	return 0;
}

struct kernfs_ops dslab_kernfs_ops = {
	.atomic_write_len		=	PAGE_SIZE,
	.write					=	dslab_file_write,
	.seq_show				=	dslab_seqfile_show,
};

static struct dentry *dslab_mount(struct file_system_type *fs_type, 
				int flags, const char *unused_dev_name, 
				void *data)
{
	struct dentry *dentry = NULL;
	struct kernfs_root *kf_root;
	struct kernfs_node *kn;
	bool new_sb;

	kf_root = kernfs_create_root(&dslab_kf_syscall_ops,
							KERNFS_ROOT_CREATE_DEACTIVATED,
							NULL);	

	if (IS_ERR(kf_root))
		goto out;

	kn = __kernfs_create_file(kf_root->kn, "dslab", FS_MODE,
							0, &dslab_kernfs_ops, NULL, NULL,
							NULL);

	if (IS_ERR(kn))
		goto destroy_root;

	dentry = kernfs_mount(fs_type, flags, kf_root,
						FS_MAGIC, &new_sb);

	if (IS_ERR(dentry) || !new_sb)
		goto destroy_kn;

	kernfs_activate(kf_root->kn);	

	goto out;

destroy_kn:
	kernfs_remove_by_name(kf_root->kn, "dslab");

destroy_root:
	kernfs_destroy_root(kf_root);
	kf_root = NULL;

out:
	return dentry;
}

static void dslab_kill_sb(struct super_block *sb)
{
	kernfs_kill_sb(sb);
}

struct file_system_type dslab_fs_type = {
	.name = FS_NAME,
	.mount = dslab_mount,
	.kill_sb = dslab_kill_sb,
};

static int __init dslab_fs_init(void)
{
	int retval;
	
	retval = register_filesystem(&dslab_fs_type);

	return retval;
}

static void __exit dslab_fs_exit(void)
{
	unregister_filesystem(&dslab_fs_type);
}

module_init(dslab_fs_init);
module_exit(dslab_fs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wang Xiaoqiang");
