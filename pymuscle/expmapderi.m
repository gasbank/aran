%
% Computes the derivatives of the exponential map rotation
%
function [R_sub, dRdv, d2Rdvdv] = expmapderi(nearzero)

syms w x y z real % quaternion
syms v1 v2 v3 theta real % exponential map parameters and its length
syms cx cy cz real
q = [w x y z]';
R = [ 1-2*y*y-2*z*z   2*x*y+2*w*z     2*x*z-2*w*y     cx;
      2*x*y-2*w*z     1-2*x*x-2*z*z   2*y*z+2*w*x     cy;
      2*x*z+2*w*y     2*y*z-2*w*x     1-2*x*x-2*y*y   cz;
      0               0               0               1   ];

% dR/dq is a (4x4)x4 tensor, that is,
% a 4 elements vector of 3x3 matrices,
% each of which is the partial derivative
% of R with respect to one of the 4 parameters
% in q(quaternion).
dRdq = sym(zeros(4,4,4));
for i=1:4
    dRdq(:,:,i) = diff(R,q(i));
end

v = [v1 v2 v3]';
if nearzero == 0
    qs = [cos(0.5*theta)   sin(0.5*theta)/theta*v1   sin(0.5*theta)/theta*v2   sin(0.5*theta)/theta*v3]';
else
    qs = [cos(0.5*theta)   0.5+theta^2/48*v1   0.5+theta^2/48*v2   0.5+theta^2/48*v3]';
end
dRdq = subs(dRdq, q, qs);

R_sub = subs(R, q, qs);

% dq/dv is 4x3 matrix.
% [ dqw/dv1   dqw/dv2   dqw/dv3;
%   dqx/dv1   dqx/dv2   dqx/dv3;
%   dqy/dv1   dqy/dv2   dqy/dv3;
%   dqz/dv1   dqz/dv2   dqz/dv3 ]
dqdv = sym(zeros(4,3));

if nearzero == 0
    % Case: theta=|v| is far from 0
    for i = 1:4
        for j = 1:3
            if i == 1
                val = -0.5*v(j)*sin(0.5*theta)/theta;
            elseif i-1 == j
                val = 0.5*v(j)^2*cos(0.5*theta)/theta^2 - v(j)^2*sin(0.5*theta)/theta^3 + sin(0.5*theta)/theta;
            else
                val = 0.5*v(i-1)*v(j)*cos(0.5*theta)/theta^2 - v(i-1)*v(j)*sin(0.5*theta)/theta^3;
            end
            dqdv(i,j) = val;
        end
    end
else
    % Case: theta=|v| is near zero value
    for i = 1:4
        for j = 1:3
            if i == 1
                val = -0.5*v(j)*(0.5-theta^2/48);
            elseif i-1 == j
                val = v(j)^2/24*(theta^2/40-1) + (0.5-theta^2/48);
            else
                val = v(i-1)*v(j)/24*(theta^2/40-1);
            end
            dqdv(i,j) = val;
        end
    end
end


% The first derivatives (dR/dv) - (4x4)x3 tensors
dRdv = sym(zeros(4,4,3));
for i = 1:3
    val = sym(zeros(4,4));
    for j = 1:4
        val = val + dRdq(:,:,j)*dqdv(j,i);
    end
    dRdv(:,:,i) = simplify(val);
end

% The second derivatives (d2R/dvdv)
dRdv_sub = subs(dRdv, theta, sqrt(v1*v1 + v2*v2 + v3*v3));
d2Rdvdv = sym(zeros(4,4,3,3));
for i = 1:3
    for j = 1:i
        d2Rdvdv(:,:,j,i) = simplify(diff(dRdv_sub(:,:,j), v(i)));
        
        % Manual simplification
        % tsq := v1*v1 + v2*v2 + v3*v3 (theta squared)
        % theta := tsq^(1/2)
        % ct := cos(theta)
        % st := sin(theta)
        syms tsq theta ct st real
        d2Rdvdv(:,:,j,i) = subs(d2Rdvdv(:,:,j,i), v1^2+v2^2+v3^2, tsq);
        d2Rdvdv(:,:,j,i) = subs(d2Rdvdv(:,:,j,i), tsq^(1/2), theta);
        d2Rdvdv(:,:,j,i) = subs(d2Rdvdv(:,:,j,i), cos(theta), ct);
        d2Rdvdv(:,:,j,i) = subs(d2Rdvdv(:,:,j,i), sin(theta), st);
        d2Rdvdv(:,:,j,i) = simplify(d2Rdvdv(:,:,j,i));
    end
end

end
