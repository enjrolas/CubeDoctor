
If your application uses MFC in a shared DLL, you will need
to redistribute the MFC DLLs. If your application is in a language
other than the operating system's locale, you will also have to
redistribute the corresponding localized resources MFC90XXX.DLL.
For more information on both of these topics, please see the section on
redistributing Visual C++ applications in MSDN documentation.

/////////////////////////////////////////////////////////////////////////////

�л������ܣ�����һЩҪ�Ľ���
1. ��vcredist_x86.exe����������
2. �����û���...���ѡ�����İ�ťѡ���˳��򣬷��򶼴����������µĳ�����Ϊ�������ϵ�����������ᶨ�ڸ��¡�
3. ��ϵͳ�Ҳ���DFU�豸ʱ��Ҫ��ʾ���������û�����������DFU����װ����
4. DFU����������������һ�¾Ͱ�װ��


���� IDB_PNG_README ���벻��ͨ��ʱ��ɾ�������µ���Ϳ�

libwdi.c
// extract the embedded binary resources
static int extract_binaries(const char* path)

/*
 * Return a wdi_device_info list of USB devices
 * parameter: driverless_only - boolean
 */
LIBWDI_EXP int LIBWDI_API wdi_create_list(struct wdi_device_info** list,
							   struct wdi_options_create_list* options);

/*
 * Release a wdi_device_info list allocated by the previous call
 */
LIBWDI_EXP int LIBWDI_API wdi_destroy_list(struct wdi_device_info* list);

/*
 * Create an inf file for a specific device
 */
LIBWDI_EXP int LIBWDI_API wdi_prepare_driver(struct wdi_device_info* device_info, const char* path,
								  const char* inf_name, struct wdi_options_prepare_driver* options);

/*
 * Install a driver for a specific device
 */
LIBWDI_EXP int LIBWDI_API wdi_install_driver(struct wdi_device_info* device_info, const char* path,
								  const char* inf_name, struct wdi_options_install_driver* options);
