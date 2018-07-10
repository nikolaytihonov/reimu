dofile("base.lua")

dir = ""
maxErrors = 10
task.setThreadCount(5)
--https://rule34.xxx/index.php?page=post&s=list&tags=TAG&pid=(p-1)*42

function getPagePosts(tag,page)
	local curl = curl_open()
	curl:setOpt(CURLOPT_URL,string.format("https://rule34.xxx/index.php?page=post&s=list&tags=%s&pid=%d",tag,(page-1)*42))
	local body,l,k = _performCurl(curl,maxErrors)
	curl:close()
	if body == nil then return nil end
	local html = tohtml(body)
	local posts = {}
	--<a id="p2825091" href="index.php?page=post&amp;s=view&amp;id=2825091"><img src="https://rule34.xxx/thumbnails/2543/thumbnail_76bd202251e9ff28dc76ac0ae24f39ff.jpg?2825091" alt="anal anal_beads anal_insertion anal_penetration animated ankle_cuffs anus areolae arms_above_head arms_up ass blush bondage bouncing_breasts breasts brown_hair crying crying_with_eyes_open dildo dotborn dripping dripping_cum dripping_pussy ejaculation eyes_rolling_back feet forced forced_nudity forced_orgasm forced_presentation helpless hitachi_magic_wand kneesocks large_breasts leg_lift legs legs_restrained_above_head machine mechanical mechanical_arm mechanical_fixation mechanical_hand naked nipples nude open_mouth orgasm orgasm_factory penetration pixel_art ponytail pussy pussy_ejaculation pussy_juice rape restrained school_uniform schoolgirl sex_machine sex_toy shiny_skin shirt_lift short_hair spiked_dildo squirting stationary_restraints stomach_bulge strapped_down tears thighs thrusting trembling twitching vibrator wrist_cuffs" border="0" title="anal anal_beads anal_insertion anal_penetration animated ankle_cuffs anus areolae arms_above_head arms_up ass blush bondage bouncing_breasts breasts brown_hair crying crying_with_eyes_open dildo dotborn dripping dripping_cum dripping_pussy ejaculation eyes_rolling_back feet forced forced_nudity forced_orgasm forced_presentation helpless hitachi_magic_wand kneesocks large_breasts leg_lift legs legs_restrained_above_head machine mechanical mechanical_arm mechanical_fixation mechanical_hand naked nipples nude open_mouth orgasm orgasm_factory penetration pixel_art ponytail pussy pussy_ejaculation pussy_juice rape restrained school_uniform schoolgirl sex_machine sex_toy shiny_skin shirt_lift short_hair spiked_dildo squirting stationary_restraints stomach_bulge strapped_down tears thighs thrusting trembling twitching vibrator wrist_cuffs score:4 rating:explicit" class="preview"></a>
	for k,v in pairs(html:toTable()) do
		if v:isTag() and v:tagName() == "a" and v:attribute("id") ~= nil then
			local id = v:attribute("id")
			if string.sub(id,1,1) == "p" then
				table.insert(posts,string.sub(id,2))
			end
		end
	end
	return posts
end

--thread function
function downloadPost(id)
	dofile("base.lua")
	local curl = curl_open()
	curl:setOpt(CURLOPT_URL,"https://rule34.xxx/index.php?page=post&s=view&id="..id)
	local body,l,k = _performCurl(curl,10)
	if body == nil then
		print("Parsing post "..id.." failed")
		return
	end

	local link = nil
	local ext = nil
	local html = tohtml(body)
	for k,v in pairs(html:toTable()) do
		if v:isTag() and v:tagName() == "img" and v:attribute("alt") ~= nil
			and v:attribute("id") == "image" then
			link = v:attribute("src")
			break
		end
	end
	if link == nil then print"Link not found!" return end
	curl:setOpt(CURLOPT_URL,link)

	for i=#link,1,-1 do
		local c = string.sub(link,i,i)
		if c == "." then
			local ext2 = string.sub(link,i)
			local l,k = string.find(ext2,"?")
			if l ~= nil and k ~= nil then
				ext = string.sub(ext2,1,k-1)
			end
			break
		end
	end

	local fname = task.getGlobal("dir").."/"..id..ext
	local f = io.open(fname,"wb")
	if f == nil then
		print("Failed to open "..fname)
		curl:close()
		return
	end

	local l,k = _performFileCurl(curl,f,10)
	if l == 0 then print(fname)
	else print("Download "..id.." failed") end
	curl:close()
end

--local posts = getPagePosts("dotborn",1)

function downloadTags(tags,pages)
	file.mkdir(tags)
	for i=1,pages do
		dir = tags.."/"..tostring(i)
		file.mkdir(dir)
		posts = getPagePosts(tags,i)
		performMultiTask(downloadPost,posts)
		print("== Page "..i.." downloaded!")
	end
end

downloadTags(args[2],tonumber(args[3]))