function src = transform1d(src, start, len, step)
    tmp = 0:len;
    W = 1/sqrt(2);
    l = len/2;
    while (l>=32)
        for i = 0:l-1
            c = src(i*2*step + start+1);
            w = src((i*2+1)*step + start+1);
            tmp(i+1) = (c+w)*W;
            tmp(i+l+1) = (c-w)*W;
        end
        for i = 0:l*2-1
            src(i*step + start+1) = tmp(i+1);
        end
        l = l/2;
    end
end