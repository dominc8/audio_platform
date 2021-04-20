function [y] = fft_4p(x)
    a = x(1) + x(3);
    b = x(1) - x(3);
    c = x(2) + x(4);
    d = -j*(x(2) - x(4));
    y(1) = a + c;
    y(2) = b + d;
    y(3) = a - c;
    y(4) = b - d;
end
