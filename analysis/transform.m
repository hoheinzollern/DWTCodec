function src = transform(src)
    for i = 0:511
        src = transform1d(src, i * 512, 512, 1);
    end
    for i = 0:511
        src = transform1d(src, i, 512, 512);
    end
end