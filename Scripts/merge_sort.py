ipt = input("Provide a list separated by whitespace: ")
ipt = ipt.split(' ')
list = []
for n in ipt:
	list.append(int(n))

def merge_sort(list):
	if len(list) <= 1:
		return list
	
	M = len(list)//2
	L, R = merge_sort(list[:M]), merge_sort(list[M:])
	
	merged = []
	i = j = 0

	while i < len(L) and j < len(R):
		if L[i] <= R[j]: 
			merged.append(L[i])
			i += 1
		else:
			merged.append(R[j])
			j += 1
	if i < len(L):
		merged += L[i:]
	elif j < len(R):
		merged += R[j:]

	return merged

print(merge_sort(list))
