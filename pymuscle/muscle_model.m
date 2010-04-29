function muscle_model()
syms XE YE ZE X1 Y1 Z1 X2 Y2 Z2 XT YT ZT real
syms c1 s1 c2 s2 c3 s3 real
CE = [XE YE ZE]';
C1 = [X1 Y1 Z1]';
C2 = [X2 Y2 Z2]';
CT = [XT YT ZT]';

A = [ 1 0 0;
      0 c1 s1;
      0 -s1 c1 ];
C1 = A*CE;

C1
end