function [yl, yr] = fft_16hist(x)
    xl = x(1:2:end);
    xr = x(2:2:end);
    yl = int16(fft_16hist_sch(xl)/64);
    yr = int16(fft_16hist_sch(xr)/64);
end

function [y] = fft_16hist_sch(x)
    w = exp(-j*pi/16) .^ (0:1:15);
    a = fft_16p(x(1:2:end));
    b = fft_16p(x(2:2:end)) .* w;
    y = abs(a+b);
end

