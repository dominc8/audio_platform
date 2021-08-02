function fir_generate(base_path)

    N = 128;

    x = (rand(1, N) - 0.5) * 2 * 256;

    Hd = get_ref_filter();
    coeff = Hd.Numerator;

    y = filter(Hd, single(x));

    save_arr(int32(x), append(base_path, 'x.dat'));
    save_arr(int32(y), append(base_path, 'y.dat'));
    save_arr(single(coeff), append(base_path, 'coeff.dat'));

end


function Hd = get_ref_filter()

    order = 10;
    Fpass = 0.45;
    Fstop = 0.55;

    h = fdesign.lowpass('n,fp,fst', order, Fpass, Fstop);

    Hd = design(h, 'firls');

    set(Hd, 'Arithmetic', 'single');

end

