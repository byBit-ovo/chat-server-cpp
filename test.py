from google import genai
import bisect

# client = genai.Client()

# response = client.models.generate_content(
#     model="gemini-2.5-pro",
#     contents="deploy的中文含义有哪些？请以json格式返回所有词性以及其对应的中文释义，不需要任何多余字符",
# )

# print(response.text)#hello

nums = [1,2,3,4,4,4,5,6,6,7]

for n in (-1, 2, 4, 6, 10):
	print(bisect.bisect_left(nums, n),end = ' ')
	# 0, 1, 3, 7, 10
print()
for n in (-1, 2, 4, 6, 10):
	print(bisect.bisect_right(nums,n), end = ' ')
	# 0, 2, 6, 9, 10
print()

example = {
	"hello": 3,
	"auuu": 4
}

print(sorted(example, key=lambda x: x[1]))


