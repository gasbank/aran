function muscle_model()

syms A x xd T real
syms KSE KPE b xrest real
syms h real

f = KSE/b*(KPE*(x-xrest) + b*xd - (1+KPE/KSE)*T + A);
Y = T;

dfdY = jacobian(f,Y)
dfdY_inv = inv(dfdY)
end