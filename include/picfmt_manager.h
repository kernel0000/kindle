
#ifndef _PIC_MANAGER_H
#define _PIC_MANAGER_H

#include <config.h>
#include <pic_operation.h>
#include <page_manager.h>
#include <file.h>

/**********************************************************************
 * 函数名称： RegisterPicFileParser
 * 功能描述： 注册"图片文件解析模块", "图片文件解析模块"就是怎么从BMP/JPG等图片文件中解析出象素数据
 * 输入参数： ptPicFileParser - 一个结构体,内含"图片文件解析模块"的操作函数
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/01/09	     V2.0	  刘鹏	      修改
 ***********************************************************************/
int RegisterPicFileParser(PT_PicFileParser ptPicFileParser);

/**********************************************************************
 * 函数名称： ShowPicFmts
 * 功能描述： 显示本程序能支持的"图片文件解析模块"
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/01/09	     V2.0	  刘鹏	      修改
 ***********************************************************************/
void ShowPicFmts(void);

/**********************************************************************
 * 函数名称： PicFmtsInit
 * 功能描述： 调用各个"图片文件解析模块"的初始化函数,就是注册它们
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/01/09	     V2.0	  刘鹏	      修改
 ***********************************************************************/
int PicFmtsInit(void);

/**********************************************************************
 * 函数名称： JPGParserInit
 * 功能描述： 注册"JPG文件解析模块"
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/01/09	     V2.0	  刘鹏	      修改
 ***********************************************************************/
int JPGParserInit(void);

/**********************************************************************
 * 函数名称： BMPParserInit
 * 功能描述： 注册"BMP文件解析模块"
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/01/09	     V2.0	  刘鹏	      修改
 ***********************************************************************/
int BMPParserInit(void);

/**********************************************************************
 * 函数名称： Parser
 * 功能描述： 根据名字取出指定的"图片文件解析模块"
 * 输入参数： pcName - 名字
 * 输出参数： 无
 * 返 回 值： NULL   - 失败,没有指定的模块, 
 *            非NULL - "图片文件解析模块"的PT_PicFileParser结构体指针
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/01/09	     V2.0	  刘鹏	      修改
 ***********************************************************************/
PT_PicFileParser Parser(char *pcName);

/**********************************************************************
 * 函数名称： GetParser
 * 功能描述： 找到能支持指定文件的"图片文件解析模块"
 * 输入参数： ptFileMap - 内含文件信息
 * 输出参数： 无
 * 返 回 值： NULL   - 失败,没有指定的模块, 
 *            非NULL - 支持该文件的"图片文件解析模块"的PT_PicFileParser结构体指针
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2016/01/09	     V2.0	  刘鹏	      修改
 ***********************************************************************/
PT_PicFileParser GetParser(PT_FileMap ptFileMap);

#endif /* _PIC_MANAGER_H */

