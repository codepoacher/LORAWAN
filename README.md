1、	编译说明
	1） 直接使用make进行编译，编译生成IA-WA200G-MCU-L083.bin
	2）	使用build.sh进行编译时，会创建一个build目录，IA-WA200G-MCU-L083.bin会
		生成在build目录下

2、	目录说明	
	build_conf		用于配置编译需要用到的文件
		chip_conf	配置芯片类型	
		hal_conf	配置hal库路径
		head_conf	配置头文件路径
		src_conf	配置.c文件路径

	build_tool		编译时用到的shell工具

	framework		frtos框架代码	
		freertos	freertos源代码
		library		使用的库源代码
		module		模块代码

	m_system		系统框架相关
		arch		硬件体系结构相关目录
			s32l083		stm32l083xx相关代码
				include		头文件目录	
				libhal		hal库目录
				linker		链接用的ld文件目录
				Src			体系结构相关的c文件
				startup		系统启动时的汇编代码
		hal_driver	驱动相关代码
		include		驱动配置头文件
	
	m_test			测试用的代码目录	

	m_user			应用代码路径

