# lua534 (reimu)
Lua Web Dumper<br>

I wrote this thing because in some period of my life i was needed in fast way to dump websites, so here it's

## Introduction
Lua534 supports multithreading. It's not coroutines, it's real threads with different lua states (for 20 threads you
have 20 lua states). Also providen htmlcxx and curl libraries

## task table
task table stands for multitasking manipulations with threads.<br>
```setThreadCount(number)``` -- sets default number of threads in payload. Used when you launch 100 jobs but need 10 threads per each payload.<br>
```setDelay(msecs)``` -- sets delay between thread spawning<br>
```setTimeOut(msecs)``` -- set timeout for thread. Used when you need close thread after some period of time if it's doesn't closed yet<br>
```getGlobal(name)``` -- returns variable from Global Lua State. Used to get some settings or variables in Thread Lua States<br>
```lockGlobal()``` -- regular mutex<br>
```unlockGlobal()``` -- regular mutex<br>

## file table
some file utils<br>
```exists(path)``` -- checks if file or dir exists<br>
```mkdir(name)``` -- creates directory<br>
```stat(path)``` -- stat C function. Returns ```struct stat``` table, like in C<br>
```size(path)``` -- returns file size<br>
```remove(path)``` -- removes file or directory<br>

## u8 table
UTF-8 and UTF-16 conversion functions<br>
```print(utf16text)``` -- prints UTF-16 text<br>
```scan(bufsize)``` -- reads UTF-16 input from console<br>
```write(file,utf16text)``` -- writes UTF-16 text to file<br>
```conv_u16(utf16text)``` -- convers UTF-16 to UTF-8<br>
```conv_u8(text)``` -- convers UTF-8 to UTF-16<br>

## global functions
```tohtml(html_text)``` -- returns html object<br>
### HTML's Object Metatable
```contentOf(child)``` -- returns content of child<br>
```toTable()``` -- returns table of html childs<br>
```byTagName(tagname)``` -- returns table of tags with name ```tagname```<br>
```getChildsOf(child)``` -- returns table of childs of ```child```<br>
## global functions
```curl_open()``` -- returns CURL handle object<br>
### CURL's Object Metatable
```close()``` -- closes handle<br>
```setOpt(CURLOPT_,arg)``` -- sets curl opt. Arg can be string or number. Supported CURLOPT_*:<br>
* CURLOPT_URL
* CURLOPT_PORT
* CURLOPT_POST
* CURLOPT_PROXY
* CURLOPT_VERBOSE
* CURLOPT_TIMEOUT
* CURLOPT_USERAGENT
* CURLOPT_POSTFIELDS
* CURLOPT_AUTOREFERER
* CURLOPT_REFERER
* CURLOPT_COOKIE
* CURLOPT_COOKIEFILE
* CURLOPT_COOKIEJAR
* CURLOPT_COOKIELIST
* CURLOPT_FOLLOWLOCATION
* CURLOPT_CONNECTTIMEOUT




```getInfo(CURLINFO_)``` -- returns curl info. Supported CURLINFO_*:<br>
* CURLINFO_RESPONSE_CODE
* CURLINFO_HTTP_CODE
## global functions
```performMultiTask(thread_function,args)``` -- Launch Lua Threads for each element in table. Notice that each thread have it own lua state,
so you must access variables from Global Lua State via ```task.getGlobal(name)```. If you did dofile in Global Lua State, you must it in Lua Thread State<br>

## Example
```lua
--<a href="/img/Lena/-4quaSExzHc.jpg" target="_blank">
dofile("base.lua")
task.setThreadCount(10)

USERAGENT = "GM9 REVCOUNCIL"
URL = "https://anonymus-lenofag.github.io"
MAXERRORS = 5

curl = curl_open()
curl:setOpt(CURLOPT_USERAGENT,USERAGENT)
curl:setOpt(CURLOPT_URL,URL)
data,res,code = _performCurl(curl,MAXERRORS)
curl:close()

function download(pic)
	dofile("base.lua")
	local l,k = pic:find("Lena/")
	local fpic = io.open("lena"..pic:sub(k),"wb")
	local curl = curl_open()

	curl:setOpt(CURLOPT_URL,task.getGlobal("URL")..pic)
	curl:setOpt(CURLOPT_USERAGENT,task.getGlobal("USERAGENT"))
	local res,code = _performFileCurl(curl,fpic,task.getGlobal("MAXERRORS"))
	if res == 0 then print(pic) end
	fpic:close()
	curl:close()
end

pics = {}
prs = tohtml(data)
for k,v in pairs(prs:toTable()) do
	local href = v:attribute("href")
	local target = v:attribute("target")
	if v:tagName() == "a" and href ~= nil 
		and target ~= nil then
		pics[#pics+1] = href
	end
end

file.mkdir("lena/")
performMultiTask(download,pics)
```

## P.S.
Also there is table of arguments ```args``` in Global Lua State.<br>
```reimu.exe myscript.lua arg1 arg2 arg3```<br>
```lua
for k,v in pairs(args) do
	print(v)
end
```


* myscript.lua
* arg1
* arg2
* arg3
