local f = io.openw(u8.conv_u8("тест.txt"),u8.conv_u8("wb"))
f:write("Привет мир!")
f:write("\n")
f:close()