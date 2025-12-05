from google import genai

client = genai.Client()

response = client.models.generate_content(
    model="gemini-2.5-pro",
    contents="deploy的中文含义有哪些？请以json格式返回所有词性以及其对应的中文释义，不需要任何多余字符",
)

print(response.text)#hello
