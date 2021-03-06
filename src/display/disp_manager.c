
#include <config.h>
#include <disp_manager.h>
#include <string.h>

static PT_DispOpr g_ptDispOprHead;//显示选项链表
static PT_DispOpr g_ptDefaultDispOpr; //默认显示选项
static PT_VideoMem g_ptVideoMemHead; //显示内存链表


/**
 * 注册一个显示选项
 * @ptDispOpr: 显示选项对象的指针
 * @return: 0
 */
int RegisterDispOpr(PT_DispOpr ptDispOpr)
{
	PT_DispOpr ptTmp;

	if (!g_ptDispOprHead) {
		g_ptDispOprHead   = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	} else {
		ptTmp = g_ptDispOprHead;
		while (ptTmp->ptNext)
			ptTmp = ptTmp->ptNext;
		ptTmp->ptNext	  = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}

	return 0;
}


/**
 * 显示当前程序支持的显示选项, 打印链表
 * @return: NULL
 */
void ShowDispOpr(void)
{
	int i = 0;
	PT_DispOpr ptTmp = g_ptDispOprHead;

	while (ptTmp) {
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


/**
 * 根据名字获取一个显示选项
 * @pcName: 显示选项名称
 * @return: 显示选项指针, 链表对应节点指针
 */
PT_DispOpr GetDispOpr(char *pcName)
{
	PT_DispOpr ptTmp = g_ptDispOprHead;
	
	while (ptTmp) {
		if (strcmp(ptTmp->name, pcName) == 0)
			return ptTmp;
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}


/**
 * 根据名称获取显示设备并初始化和清屏
 * @name: 显示选项名称
 * return: NULL
 */
void SelectAndInitDefaultDispDev(char *name)
{
	g_ptDefaultDispOpr = GetDispOpr(name);
	if (g_ptDefaultDispOpr) {
		g_ptDefaultDispOpr->DeviceInit();
		g_ptDefaultDispOpr->CleanScreen(0);
	}
}


/**
 * 获取一个默认显示选项
 * 		程序在这之前使用SelectAndInitDefaultDispDev初始化了
 * 		全局变量g_ptDefaultDispOpr
 * @return: 全局变量, 默认显示选项
 */
PT_DispOpr GetDefaultDispDev(void)
{
	return g_ptDefaultDispOpr;
}


/**
 * 获取显示设备的分辨率和pbb
 * @piXres: 返回x分辨率
 * @piYres: 返回y分辨率
 * @piBpp: 返回pbb(一个象素用多少位来表示)
 * @return: 0 成功, -1 失败
 */
int GetDispResolution(int *piXres, int *piYres, int *piBpp)
{
	if (g_ptDefaultDispOpr) {
		*piXres = g_ptDefaultDispOpr->iXres;
		*piYres = g_ptDefaultDispOpr->iYres;
		*piBpp  = g_ptDefaultDispOpr->iBpp;
		return 0;
	} else {
		return -1;
	}
}


/**
 * 分配显示缓存
 * 		为了加快显示速度, 在显示前事先构造好显示数据, 
 * 		显示时候直接, 显示时候直接将缓存内存拷贝到设备显存
 * @iNum: 内存块的数目, 一块一个屏幕
 * @return: 0 成功, -1失败
 */
int AllocVideoMem(int iNum)
{
	int i;

	int iXres = 0;
	int iYres = 0;
	int iBpp  = 0;

	int iVMSize;
	int iLineBytes;

	PT_VideoMem ptNew;

	/* 确定VideoMem的大小
	 */
	GetDispResolution(&iXres, &iYres, &iBpp);
	iVMSize = iXres * iYres * iBpp / 8; //一块内存的字节数
	iLineBytes = iXres * iBpp / 8; //一行字节数


	/* 先把设备本身的framebuffer放入链表
	 * 分配一个T_VideoMem结构体, 注意我们没有分配里面的tPixelDatas.aucPixelDatas
	 * 而是让tPixelDatas.aucPixelDatas指向显示设备的framebuffer
	 */
	ptNew = malloc(sizeof(T_VideoMem));
	if (ptNew == NULL)
		return -1;

	/* 指向framebuffer */
	ptNew->tPixelDatas.aucPixelDatas = g_ptDefaultDispOpr->pucDispMem;
	
	ptNew->iID = 0;
	ptNew->bDevFrameBuffer = 1;        /* 表示这个VideoMem是设备本身的framebuffer, 而不是用作缓存作用的VideoMem */
	ptNew->eVideoMemState  = VMS_FREE; //空闲状态
	ptNew->ePicState	   = PS_BLANK; //图片状态
	ptNew->tPixelDatas.iWidth  = iXres;
	ptNew->tPixelDatas.iHeight = iYres;
	ptNew->tPixelDatas.iBpp    = iBpp;
	ptNew->tPixelDatas.iLineBytes  = iLineBytes; //行字节数
	ptNew->tPixelDatas.iTotalBytes = iVMSize; //总字节数

	if (iNum != 0) {
		/* 如果下面要分配用于缓存的VideoMem, 
		 * 把设备本身framebuffer对应的VideoMem状态设置为VMS_USED_FOR_CUR,
		 * 表示这个VideoMem不会被作为缓存分配出去
		 */
		ptNew->eVideoMemState = VMS_USED_FOR_CUR;
	}
	
	/* 放入链表 */
	ptNew->ptNext = g_ptVideoMemHead;
	g_ptVideoMemHead = ptNew;
	

	/*tPixelDatas
	 * 分配用于缓存的VideoMem
	 */
	for (i = 0; i < iNum; i++) {
		/* 分配T_VideoMem结构体本身和"跟framebuffer同样大小的缓存" */
		ptNew = malloc(sizeof(T_VideoMem) + iVMSize);
		if (ptNew == NULL) 
			return -1; 
		/* 在T_VideoMem结构体里记录上面分配的"跟framebuffer同样大小的缓存" */
		ptNew->tPixelDatas.aucPixelDatas = (unsigned char *)(ptNew + 1);

		ptNew->iID = 0;
		ptNew->bDevFrameBuffer = 0;
		ptNew->eVideoMemState = VMS_FREE;
		ptNew->ePicState      = PS_BLANK;
		ptNew->tPixelDatas.iWidth  = iXres;
		ptNew->tPixelDatas.iHeight = iYres;
		ptNew->tPixelDatas.iBpp    = iBpp;
		ptNew->tPixelDatas.iLineBytes = iLineBytes;
		ptNew->tPixelDatas.iTotalBytes = iVMSize;

		/* 放入链表 */
		ptNew->ptNext = g_ptVideoMemHead;
		g_ptVideoMemHead = ptNew;
	}
	
	return 0;
}


/**
 * 取一块空闲内存
 * @iID: 指定ID
 * @bCur: 1表示当前程序马上要使用VideoMem,无论如何都要返回一个VideoMem
 *        0表示这是为了改进性能而提前取得VideoMem,不是必需的
 * @return: NULL 没有可用mem
 * 			非NULL 成功, 并返回mem结构指针
 */
PT_VideoMem GetVideoMem(int iID, int bCur)
{
	PT_VideoMem ptTmp = g_ptVideoMemHead;
	
	/* 1. 优先: 取出空闲的、ID相同的videomem */
	while (ptTmp) {
		if ((ptTmp->eVideoMemState == VMS_FREE) && (ptTmp->iID == iID)) {
			ptTmp->eVideoMemState = bCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}

	/* 2. 如果前面不成功, 取出一个空闲的并且里面没有数据(ptVideoMem->ePicState = PS_BLANK)的VideoMem */
	ptTmp = g_ptVideoMemHead;
	while (ptTmp) {
		if ((ptTmp->eVideoMemState == VMS_FREE) && (ptTmp->ePicState == PS_BLANK)) {
			ptTmp->iID = iID;
			ptTmp->eVideoMemState = bCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	
	/* 3. 如果前面不成功: 取出任意一个空闲的VideoMem */
	ptTmp = g_ptVideoMemHead;
	while (ptTmp) {
		if (ptTmp->eVideoMemState == VMS_FREE) {
			ptTmp->iID = iID;
			ptTmp->ePicState = PS_BLANK;
			ptTmp->eVideoMemState = bCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}

    /* 4. 如果没有空闲的VideoMem并且bCur为1, 则取出任意一个VideoMem(不管它是否空闲) */
    if (bCur) {
    	ptTmp = g_ptVideoMemHead;
    	ptTmp->iID = iID;
    	ptTmp->ePicState = PS_BLANK;
    	ptTmp->eVideoMemState = bCur ? VMS_USED_FOR_CUR : VMS_USED_FOR_PREPARE;
    	return ptTmp;
    }
    
	return NULL;
}


/**
 * 将不使用的mem放入内存链表设置伟空闲
 * 		Get获取, 使用完用Put释放到链表
 * @ptVideoMem: 内存节点指针
 */
void PutVideoMem(PT_VideoMem ptVideoMem)
{
	ptVideoMem->eVideoMemState = VMS_FREE;  /* 设置VideoMem状态为空闲 */
    if (ptVideoMem->iID == -1) 
        ptVideoMem->ePicState = PS_BLANK;  /* 表示里面的数据没有用了 */ 
}


/**
 * 获得显示设备的显存, 在该显存上操作就可以直接在LCD上显示出来
 * @return: 显存结构体指针
 */
PT_VideoMem GetDevVideoMem(void)
{
	PT_VideoMem ptTmp = g_ptVideoMemHead;
	
	while (ptTmp) {
		if (ptTmp->bDevFrameBuffer)
			return ptTmp;
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}


/**
 * 把VideoMem中内存全部清为某种颜色
 * @ptVideoMem: mem指针
 * @dwColor: 颜色
 */
void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	unsigned char *pucVM;
	unsigned short *pwVM16bpp;
	unsigned int *pdwVM32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;
	int i = 0;

	pucVM	   = ptVideoMem->tPixelDatas.aucPixelDatas;
	pwVM16bpp  = (unsigned short *)pucVM;
	pdwVM32bpp = (unsigned int *)pucVM;

	switch (ptVideoMem->tPixelDatas.iBpp) {
		case 8: {
			//按字节为单位设置
			memset(pucVM, dwColor, ptVideoMem->tPixelDatas.iTotalBytes);
			break;
		}
		case 16: {
			/* 先根据32位的dwColor构造出16位的wColor16bpp */
			iRed   = (dwColor >> (16+3)) & 0x1f;//取高22-26共5位
			iGreen = (dwColor >> (8+2)) & 0x3f;//10-15共6位
			iBlue  = (dwColor >> 3) & 0x1f;//3-7共5位
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			while (i < ptVideoMem->tPixelDatas.iTotalBytes) {
				*pwVM16bpp	= wColor16bpp;//每个16位都是这个颜色
				pwVM16bpp++;//内存指针后移
				i += 2;//short每2字节为单位
			}
			break;
		}
		case 32: {
			while (i < ptVideoMem->tPixelDatas.iTotalBytes) {
				*pdwVM32bpp = dwColor;
				pdwVM32bpp++;
				i += 4; //32位以四字节为单位
			}
			break;
		}
		default: {
			DBG_PRINTF("can't support %d bpp\n", ptVideoMem->tPixelDatas.iBpp);
			return;
		}
	}

}


/**
 * 将显示内存指定区域显设置为某种颜色
 * @ptVideoMem: 要操作的内存
 * @ptLayout: 矩形区域, 左上角右下角坐标
 * @dwColor: 设置的颜色(传入一个字32位的颜色)
 */
void ClearVideoMemRegion(PT_VideoMem ptVideoMem, PT_Layout ptLayout, unsigned int dwColor)
{
	unsigned char *pucVM;
	unsigned short *pwVM16bpp;
	unsigned int *pdwVM32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;
	int iX;
	int iY;
    int iLineBytesClear;
    int i;
	//计算指针初始位置, 就是layout第一个像素点指针
	//内存基址+顶部y*行字节数+左部x*像素字节(Layout前面半行)
	pucVM	   = ptVideoMem->tPixelDatas.aucPixelDatas \
				+ ptLayout->iTopLeftY * ptVideoMem->tPixelDatas.iLineBytes \
				+ ptLayout->iTopLeftX * ptVideoMem->tPixelDatas.iBpp / 8;

	pwVM16bpp  = (unsigned short *)pucVM;//16位一像素指针
	pdwVM32bpp = (unsigned int *)pucVM;//32位一像素指针
	//Layout一行字节数
    iLineBytesClear = (ptLayout->iBotRightX - ptLayout->iTopLeftX + 1) * ptVideoMem->tPixelDatas.iBpp / 8;

	switch (ptVideoMem->tPixelDatas.iBpp) {
		case 8: {
			//从矩形的左上y下降到矩形右下y, 设置每个layout行像素颜色
            for (iY = ptLayout->iTopLeftY; iY <= ptLayout->iBotRightY; iY++) {
    			memset(pucVM, dwColor, iLineBytesClear);
                pucVM += ptVideoMem->tPixelDatas.iLineBytes; //指针每次移动一行个字节
            }
			break;
		}
		case 16: {
			/* 先根据32位的dwColor构造出16位的wColor16bpp */
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
            for (iY = ptLayout->iTopLeftY; iY <= ptLayout->iBotRightY; iY++) {
                i = 0;
				//每一个像素都是一个short
                for (iX = ptLayout->iTopLeftX; iX <= ptLayout->iBotRightX; iX++) 
    				pwVM16bpp[i++]	= wColor16bpp;
    			//指针运算, 强转位32位加上一行字节数, 指向下一行??
                pwVM16bpp = (unsigned short *)((unsigned int)pwVM16bpp + ptVideoMem->tPixelDatas.iLineBytes);
            }
			break;
		}
		case 32: {
            for (iY = ptLayout->iTopLeftY; iY <= ptLayout->iBotRightY; iY++) {
                i = 0;
				//每个像素都是一个字, 4字节
                for (iX = ptLayout->iTopLeftX; iX <= ptLayout->iBotRightX; iX++) 
    				pdwVM32bpp[i++]	= dwColor;

                pdwVM32bpp = (unsigned int *)((unsigned int)pdwVM32bpp + ptVideoMem->tPixelDatas.iLineBytes);
            }
			break;
		}
		default: {
			DBG_PRINTF("can't support %d bpp\n", ptVideoMem->tPixelDatas.iBpp);
			return;
		}
	}

}


/**
 * 注册显示设备
 * @return: 错误码
 */
int DisplayInit(void)
{
	int iError;
	iError = FBInit();

	return iError;
}

