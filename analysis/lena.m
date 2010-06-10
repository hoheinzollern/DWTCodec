fhan = fopen('lena.bmp', 'r');
fseek(fhan, 1078, -1);
src = fread(fhan, 512*512, 'uchar');
dwt = transform(src);
sdwt = (dwt./max(dwt)+1)*2048;
count = zeros(4096,1);
for i=1:512*512
    count(round(sdwt(i))) = count(round(sdwt(i)))+1;
end
plot(count);