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