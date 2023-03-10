#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <uci.h>
staticstruct uci_context * ctx = NULL; //定義一個UCI上下文的靜態變數.
/********************************************* 
*   載入配置檔案,並遍歷Section. 
*/
bool load_config()  
{  
    struct uci_package * pkg = NULL;  
    struct uci_element *e;  
    ctx = uci_alloc_context(); // 申請一個UCI上下文.
    if (UCI_OK != uci_load(ctx, "wireless", &pkg))  
        goto cleanup; //如果開啟UCI檔案失敗,則跳到末尾 清理 UCI 上下文.
    /*遍歷UCI的每一個節*/
    uci_foreach_element(&pkg->sections, e)  
    {  
        struct uci_section *s = uci_to_section(e);  
        // 將一個 element 轉換為 section型別, 如果節點有名字,則 s->anonymous 為 false.
        // 此時通過 s->e->name 來獲取.
        // 此時 您可以通過 uci_lookup_option()來獲取 當前節下的一個值.
        if (NULL != (value = uci_lookup_option_string(ctx, s, "ipaddr")))  
        {  
            ssid = strdup(value) //如果您想持有該變數值，一定要拷貝一份。當 pkg銷燬後value的記憶體會被釋放。
        }  
        // 如果您不確定是 string型別 可以先使用 uci_lookup_option() 函式得到Option 然後再判斷.
        // Option 的型別有 UCI_TYPE_STRING 和 UCI_TYPE_LIST 兩種.
    }  
    uci_unload(ctx, pkg); // 釋放 pkg 
	uci_free_context(ctx);  
	
cleanup:  
    uci_free_context(ctx);  
    ctx = NULL;  
}  


struct uci_context * ctx = uci_alloc_context(); //申請上下文
struct uci_ptr ptr ={  
    .package = "wireless",  
    .section = "wifi1",  
    .option = "channel",  
    .value = "auto",  
};  
uci_set(ctx,&ptr); //寫入配置
uci_add_list(ctx,&ptr)  // 新增一個list 值
uci_delete(ctx,&ptr)    // 刪除一個option值
uci_commit(ctx, &ptr.p, false); //提交儲存更改
uci_unload(ctx,ptr.p); //解除安裝包
uci_free_context(ctx); //釋放上下文


