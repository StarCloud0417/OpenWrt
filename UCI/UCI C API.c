#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <uci.h>
/*
 *在配置文件中新增一個節點
 *(在wireless文件中新增一個ssid節點)
 */
int add_new_section(void)
{	
	char acSsid[32] = {'0'};   //存儲輸入的ssid
	char acPasswd[32] = {'0'};  //ssid對應的密碼
	char acEncryption[8] = {'0'};
	
	struct uci_context *ctx = NULL;
	struct uci_package *pkg = NULL;
	
	//申請一個uci上下文
	ctx = uci_alloc_context();
	
	// 打開配置uci文件
	if(UCI_OK != uci_load(ctx, "wireless", &pkg))
		return -1;
	
	//添加新的節點
	if((pkg = uci_lookup_package(ctx, "wireless")) != NULL)
	{
		struct uci_ptr ptr = {
			.p = pkg
		};
		
		// 在打開的uci配置文件中新增節點配置 
		uci_add_section(ctx, pkg, "wifi-iface", &ptr.s);
		
		ptr.o = NULL;
		ptr.option = "device";
		ptr.value = "radio0";   
		uci_set(ctx, &ptr);
		
		ptr.o = NULL;
		ptr.option = "network";
		ptr.value = "lan";
		uci_set(ctx, &ptr);
		
		ptr.o = NULL;
		ptr.option = "mode";
		ptr.value = "ap";   //工作方式下，網卡的默認工作模式爲ap
		uci_set(ctx, &ptr);
		
		ptr.o = NULL;
		ptr.option = "encryption";
		ptr.value = acEncryption;
		uci_set(ctx, &ptr);
		
		ptr.o = NULL;
		ptr.option = "ssid";
		ptr.value = acSsid;
		uci_set(ctx, &ptr);
		
		if(strcmp(acEncryption, "none"))
		{
			ptr.o = NULL;
			ptr.option = "key";
			ptr.value = acPasswd;
			uci_set(ctx, &ptr);
		}
		
		uci_commit(ctx, &ptr.p, false);
		uci_unload(ctx, ptr.p);
	}
	uci_free_context(ctx);
	return 0;
}

/*
 * 按照節點的名稱刪除該節點
 * 即刪除一個ssid
 */
int delete_section(const char *acSection)
{
    int ret;
    struct uci_package* pkg;
    struct uci_section* sec;
    struct uci_ptr ptr;
    char* err_str = NULL;
    char err_msg[256];
	struct uci_context *ctx = NULL;
	
	
    if(ctx == NULL)
    {
        ctx = uci_alloc_context();  //申請uci上下文
        if(ctx==NULL)
        {
            snprintf(err_msg, sizeof(err_msg), "Failed to alloc uci context");
            fprintf(stderr, "[%s] %s\n", __func__, err_msg);
            return -1;
        }
    }

    ret = uci_load(ctx,"wireless",&pkg);
    if(ret != 0 || pkg == NULL)
    {
        snprintf(err_msg, sizeof(err_msg), "Failed to load package: '%s' with error", "/etc/config/wireless");
        goto error_pkg;
    }

    sec = uci_lookup_section(ctx, pkg, acSection); //查找節點
    if(sec != NULL)
    {
        memset(&ptr, 0, sizeof(struct uci_ptr));
        ptr.package = "wireless";
        ptr.section = acSection;
        ptr.p = pkg;
        ptr.s = sec;

        ret = uci_delete(ctx, &ptr);
        if(ret != 0)
        {
            snprintf(err_msg, sizeof(err_msg), "Failed to delete section: '%s' with error", acSection);
            goto error_uci;
        }

        uci_save(ctx, pkg);
        uci_commit(ctx, &pkg, false);
    }

        uci_unload(ctx, pkg);
	uci_free_context(ctx);
	
	return 0;
	
error_uci:
    uci_unload(ctx, pkg);
error_pkg:
    uci_get_errorstr(ctx, &err_str, err_msg);
    fprintf(stderr, "[%s] %s\n", __func__, err_str);
    free(err_str);
    return -1;
}
/*
 *刪除某一個節點的一個option選項
 */
static int uci_delete_option(const char* section,const char* option)
{
    int ret;
    struct uci_package* pkg;
	struct uci_context *ctx = NULL;
    struct uci_ptr ptr;
    char* err_str = NULL;
    char err_msg[256];

    if(ctx == NULL)
    {
        ctx = uci_alloc_context();
        if(ctx == NULL)
        {
            printf("ctx alloc failed!\n");
            return -1;
        }
    }

    ret = uci_load(ctx,"wireless",&pkg);
    if(ret != 0 || pkg == NULL)
    {
        snprintf(err_msg, sizeof(err_msg), "Failed to load package: '%s' with error", "etc/config/wireless");
		printf("%s\n", err_msg);
        goto error_pkg;
    }
    
    memset(&ptr,0,sizeof(struct uci_ptr));

    ptr.package = "wireless";
    ptr.section = section;
    ptr.option = option;

    ret = uci_delete(ctx,&ptr);

    uci_save(ctx, pkg);
    uci_commit(ctx, &pkg, false);
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
    return 0;

error_pkg:
    uci_get_errorstr(ctx, &err_str, err_msg);
    free(err_str);
    return -1;
}

/*
 *修改節點option值
 */
static int uci_set_option_string(const char *section, const char *option, const char *value)
{
    int ret;
    struct uci_context *ctx = NULL;
    struct uci_package* pkg;
    struct uci_section* sec;
    struct uci_ptr ptr;
    char* err_str = NULL;
    char err_msg[256] = {'\0'};

    if(ctx == NULL)
    {
        ctx = uci_alloc_context();  //申請一個uci上下文
        if(ctx==NULL)
        {
            snprintf(err_msg, sizeof(err_msg),"Failed to alloc uci context");
            return -1;
        }
    }

    ret = uci_load(ctx, "wireless", &pkg);
    if(ret !=0 || pkg == NULL)
    {
        snprintf(err_msg, sizeof(err_msg),"Failed to load package: '%s' with error","/etc/config/wireless");
        goto error_pkg;
    }

    sec = uci_lookup_section(ctx, pkg, section);
    if(sec == NULL)
    {
        snprintf(err_msg, sizeof(err_msg), "Failed to find section: '%s'", section);
        goto error_msg;
    }

    memset(&ptr, 0, sizeof(struct uci_ptr));

    ptr.package = "wireless";
    ptr.section = section;
	ptr.option = option;
	ptr.value = value;
	
    ptr.p = pkg;
    ptr.s = sec;

    ret = uci_set(ctx, &ptr);
    if(ret != 0)
    {
        snprintf(err_msg, sizeof(err_msg), "Failed to set option: '%s' with error", option);
        goto error_uci;
    }

    uci_save(ctx, pkg);
    uci_commit(ctx, &pkg, false);
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
    return 0;

error_uci:
    uci_unload(ctx, pkg);
error_pkg:
    uci_get_errorstr(ctx, &err_str, err_msg);
    free(err_str);
    return -1;
error_msg:
    uci_unload(ctx, pkg);
    return -1;
}




