function biquad_generate(base_path)

    N = 128;

    x = (rand(1, N) - 0.5) * 2 * 256;

    Hd = get_ref_filter();
    % Negate denominator to match CMSIS-DSP implementation
    coeff = Hd.sosMatrix(:, [1, 2, 3, 5, 6]) .* [1, 1, 1, -1, -1];
    coeff = serialize_matrix_by_rows(coeff);

    y = filter(Hd, single(x));

    save_arr(int32(x), append(base_path, 'x.dat'));
    save_arr(int32(y), append(base_path, 'y.dat'));
    save_arr(single(coeff), append(base_path, 'coeff.dat'));

end


function Hd = get_ref_filter()

    N     = 20;     % Order
    Fpass = 0.45;  % Passband Frequency
    Fstop = 0.55;  % Stopband Frequency
    Apass = 1;     % Passband Ripple (dB)

    h = fdesign.lowpass('n,fp,fst,ap', N, Fpass, Fstop, Apass);

    Hd = design(h, 'ellip', ...
        'FilterStructure', 'df2tsos', ...
        'SOSScaleNorm', 'Linf');

    set(Hd, 'Arithmetic', 'single');

end

function coeff = serialize_matrix_by_rows(m)
    coeff = transpose(m);
    coeff = coeff(:);
end

