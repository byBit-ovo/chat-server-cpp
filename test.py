from google import genai

client = genai.Client()

response = client.models.generate_content(
    model="gemini-2.5-pro",
    contents="God",
)

print(response.text)

# from google import genai

# # 确保您的 API Key 已经通过环境变量或参数传入 Client()
# try:
#     client = genai.Client()
# except ValueError as e:
#     print(f"初始化客户端失败，请检查 GOOGLE_API_KEY 环境变量是否设置: {e}")
#     exit()

# # 获取当前 Key 可用的模型列表
# try:
#     print("正在查询您 Key 的可用模型...")
#     available_models = client.models.list()

#     # 打印出所有包含 'gemini' 的模型，重点关注高级版本
#     print("\n--- 可用模型列表 ---")
    
#     found_advanced = False
    
#     for model in available_models:
#         model_name = model.name
        
#         # 筛选出 Gemini 模型，尤其是 Pro/Advanced 版本
#         if "gemini" in model_name:
#             print(f"✅ {model_name}")
#             if "pro" in model_name or "advanced" in model_name or "1.5" in model_name:
#                 found_advanced = True
                
#     if found_advanced:
#         print("\n🎉 恭喜！列表中包含 'Pro' 或 '1.5' 等高级模型，您的 Key 具有访问权限。")
#     else:
#         print("\n⚠️ 列表中未发现高级模型。这可能意味着您的 Key 尚未关联到支持高级模型的计费账户或区域。")

# except Exception as e:
#     print(f"\n查询失败。请确认您的网络连接和 API Key 有效性。\n错误详情: {e}")