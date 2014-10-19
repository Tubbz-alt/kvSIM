/*
 * kvSIM patch for Siemens 6688i(SL45i) v55lg8
 *
 * Copyright (C) 2004-2007 Konca Fung.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

/* Author: Konca Fung <konca@tom.com>. http://www.konca.com */


#include <stdio.h>

#include "SlckOsFunc.h"
#include "SlckOsVar.h"
#include "kvSIM_var.h"


#pragma romdata

#define KVSIM_MENU_ITEM_COUNT   8
KV_SIM_CARD_DATA *g_SimData = (KV_SIM_CARD_DATA*)(KV_SIM_VIRTUAL_CARDS_ADDR);

unsigned int  OnKeyPress(void *pWindowHandle, void *pKeyInfoStru);
void OnRefresh(void *pItemObj, long lItemIdx);

#define MENU_IMAGE_RADIO_FULL ((COMMON_DATA_STRU *)0xE679DE)
#define MENU_IMAGE_RADIO_NULL ((COMMON_DATA_STRU *)0xE679E2)
#define MENU_IMAGE_CHECK_FULL ((COMMON_DATA_STRU *)0xE679D6)
#define MENU_IMAGE_CHECK_NULL ((COMMON_DATA_STRU *)0xE679DA)

#define WSTRING_HEADER_SIZE 20


WINDOW_CLASS KVSIM_WindowClass = {
    NULL,
    (void*)(OnKeyPress),                // �����ص�����
    NULL,
    NULL,
    (COMMON_DATA_STRU*)0xF2BCE8,        // ����������Ϣ(Internet -> Profile)
    (COMMON_DATA_STRU*)0xF2BCD6,        // ���������Ϣ��(Internet -> Profile)
    1,                      // Window Type
    0,                      // Unknown
    (void*)(OnRefresh),        // ĳ�˵��ˢ��ʱ�Ļص�����
    NULL,                   // �˵���ṹ�б�
    NULL,                   // �˵��ѡ�еĺ����б��ָ��
    0                       // MenuItemCount
};

FRAME_CLASS  KVSIM_FrameClass = {
    0,
    0,
    101,                                // ��Ŀ���
    12,                                 // ��Ŀ�߶�
    (COMMON_DATA_STRU*)0xD1D26A,        // ͼ������
    KVSIM_APP_TITLE_STRING_ID,          // ����StringID
    0x7FFF
};


/*
Return: 
 0xFFFF - �����Ѵ������˳�����(���������ǿ���˳�)
      1 - �����Ѵ����˳��˴���
      0 - ����δ������ϵͳĬ�ϴ���
*/
unsigned int OnKeyPress(void *pWindowHandle, void *pKeyInfoStru)
{
    int nKeyValue;
    int nMenuIndex;

#if 0
    int loop;
    unsigned char *pMyDst = (unsigned char*)0x31980;
    unsigned char *pMySrc = (unsigned char*)pKeyInfoStru;
    for(loop = 0; loop < 32; loop++)
    {
        *pMyDst++ = *pMySrc++;
    }
#endif

    // ��15��int�ǰ���ֵ
    nKeyValue = ((int*)pKeyInfoStru)[15];

#if 1
    // �û����� "ѡ��" ��ʱ
    if (0x04 == nKeyValue)
    {
        nMenuIndex = (int)GetSelectMenuIdx(pWindowHandle);
        
        // ѡ��ǵ�ǰ��ʱ
        if (nMenuIndex != KVSIM_SEL_CARD_IDX)
        {
            // ѡ����ʵ������ѡ����IMSI�ǿյ����⿨
            if ((0 == nMenuIndex)
             || ((nMenuIndex > 0) && (0 != g_SimData[nMenuIndex - 1].szImsi[0])))
            {
                KVSIM_SEL_CARD_IDX = (unsigned char)nMenuIndex;
                //KVSIM_CUR_CARD_IDX = KVSIM_SEL_CARD_IDX; // KoncaTest
                SendRebootMsg();
                return 0;
            }
        }
    }
#endif

    return 0;
}

void OnRefresh(void *pMenuItemObj, long lItemIdx)
{
    WSTRING_STRU *pWString;
    void *pFatherWindowHandle;
    void *pMenuTextHandle;
    void *pCardName;
    int nTextLen;

    unsigned char szWStringHeader[WSTRING_HEADER_SIZE];

    #define MY_WSTRING_CHAR_COUNT 30
    unsigned char szTempCardName[KV_CARD_DATA_NAME_LEN+8];
    unsigned char szWStringBuffer[MY_WSTRING_CHAR_COUNT * 2];

    if (lItemIdx >= KVSIM_MENU_ITEM_COUNT)
    {
        return;
    }

    if (lItemIdx == 0)
    {
        pCardName = (unsigned char*)KV_SIM_TRUE_CARD_NAME_ADDR;
    }
    else
    {
        // ������轫������1
        pCardName = g_SimData[lItemIdx - 1].szSimName;
    }

    // Add SeqNo to the Card Name
    szTempCardName[0] = '0' + lItemIdx + 1;
    szTempCardName[1] = '-';
    LIB_Memcpy(&szTempCardName[2], pCardName, KV_CARD_DATA_NAME_LEN);
    
    // ����ַ�����
    nTextLen = STRLEN(szTempCardName);
    if (nTextLen >= MY_WSTRING_CHAR_COUNT)
    {
        return;
    }

    // ����һ��WString
    pWString = (WSTRING_STRU*)CreateWString(szWStringHeader,
                                            szWStringBuffer,
                                            MY_WSTRING_CHAR_COUNT);

    UTF8ToWString(pWString, szTempCardName, nTextLen);

    // �����˵��ı�
    pMenuTextHandle = CreateMenuText(pMenuItemObj,
                                     pWString->pBuffer->nLen);

    SetMenuTextByHandle(pMenuTextHandle, pWString);
    
    // ȡ�ø����ھ��
    pFatherWindowHandle = GetFahterWindowHandle(pMenuItemObj);

    // ѡ��������ͼ��
    SetItemImage(pMenuItemObj,
                 pFatherWindowHandle,
                 (KVSIM_CUR_CARD_IDX == lItemIdx) ? MENU_IMAGE_RADIO_FULL : MENU_IMAGE_RADIO_NULL);

    // �������
    SetItemSoftKey(pMenuItemObj, (COMMON_DATA_STRU *)0xF2BCDC);

    // ���²˵�
    RefreshMenuItem(pMenuItemObj,
                    pFatherWindowHandle,
                    pMenuTextHandle,
                    lItemIdx);

    return;
}

void KVSIM_MenuEntrance(void)
{
    void *pWindowObj;
    
    pWindowObj = MMI_AllocObject((void*)MMI_Malloc, (void*)MMI_Free);
    //MMI_ObjectInit(pWindowObj, NULL);
    InitWindowClass(pWindowObj, (COMMON_DATA_STRU*)(&KVSIM_WindowClass));

    // ������Ŀ����
    SetWindowItemCount(pWindowObj, 8);

    // ����λ�����ĵĲ˵���(��ѡ��)
    SetSelMenu(pWindowObj, KVSIM_CUR_CARD_IDX);

    ClearStringID(KVSIM_APP_TITLE_STRING_ID, 0);
    StoreASCStringAsID(KVSIM_APP_TITLE_STRING_ID,
                       KVSIM_TITLE_WITH_VERSION,
                       0);
    

    // Fill it later
    InitFrameClass(pWindowObj,
                   (COMMON_DATA_STRU*)(&KVSIM_FrameClass),
                   (void*)MMI_Malloc);
    
    // ����ȫ������
    CreateWnd_FullScreen(pWindowObj);
}

char DefProviderName[] = "My Network";
void ReplaceProviderName(void *pOutput)
{
    unsigned char *pCardName;
    
    // ʹ����ʵ����ǰ���ų�����Χʱ�����޸���Ӫ������
    if ((INDEX_OF_TRUE_SIM_CARD == KVSIM_CUR_CARD_IDX)
     || (KVSIM_CUR_CARD_IDX > LAST_INDEX_OF_CARDS))
    {
        return;
    }
    
    // ������轫������1
    pCardName = g_SimData[KVSIM_CUR_CARD_IDX - 1].szSimName;
    
    // ת�������
    UTF8ToWString(pOutput, pCardName, STRLEN(pCardName));
}

