function fft_16hist_generate(base_path)
    fs = 48000;
    dt = 1/fs;
    fft_size = 16;
    channel_size = 2*fft_size;
    t = 0:dt:(channel_size - 1)*dt;
    
    % audio samples from some music file
    audio_data = [
   -0.0060   -0.0127;
    0.0281    0.0083;
    0.0507    0.0215;
    0.0558    0.0235;
    0.0311    0.0125;
   -0.0182   -0.0104;
   -0.0544   -0.0294;
   -0.0483   -0.0263;
   -0.0145   -0.0080;
    0.0073    0.0016;
   -0.0034   -0.0053;
   -0.0298   -0.0163;
   -0.0363   -0.0172;
   -0.0099   -0.0039;
    0.0247    0.0143;
    0.0374    0.0220;
    0.0237    0.0160;
    0.0002    0.0065;
   -0.0204   -0.0017;
   -0.0380   -0.0104;
   -0.0522   -0.0162;
   -0.0563   -0.0159;
   -0.0540   -0.0144;
   -0.0532   -0.0143;
   -0.0414   -0.0070;
   -0.0025    0.0139;
    0.0504    0.0406;
    0.0816    0.0566;
    0.0668    0.0509;
    0.0147    0.0276;
   -0.0382    0.0026;
   -0.0653   -0.0120
   ];
   
    x = zeros(1, channel_size*2, 'single');
    x(1:2:end) = audio_data(:, 1);
    x(2:2:end) = audio_data(:, 2);

    x = single(int16(x * 2^22) * 256);  % Convert to 24bit samples shifted by 8 bits

    [yl, yr] = fft_16hist(x);
%     yl_ref = abs(fft(x(1:2:end)));
%     yr_ref = abs(fft(x(2:2:end)));
%     int16(yl_ref(1:16)/64)
%     int16(yr_ref(1:16)/64)

    save_arr(int32(x), append(base_path, 'x.dat'));
    save_arr(yl, append(base_path, 'yl.dat'));
    save_arr(yr, append(base_path, 'yr.dat'));

end
