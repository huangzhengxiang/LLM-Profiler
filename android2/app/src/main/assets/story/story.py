import os

if __name__=="__main__":
    f = open("story.txt", "rt")
    content = f.read()
    f.close()
    i = 0
    while(i<len(content)):
        if (content[i]=="\n"):
            if (i+1<len(content) and content[i+1]!="\n"):
                space = " " if content[i-1]!= "\n" else ""
                content = content[:i] + space + content[i+1:] # delete content i
        i+=1
        if (i%1000==0):
            print("*",end="")
    f = open("story.txt", "wt")
    print(content, file=f)
    print()