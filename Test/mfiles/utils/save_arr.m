function save_arr(x, filename)
    fid = fopen(filename, 'w');
    fwrite(fid, x, class(x));
    fclose(fid);
end
