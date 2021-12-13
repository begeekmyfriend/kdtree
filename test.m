clear;clc;close all;


%% ²âÊÔkdÊ÷´úÂë
e = 0;
k = 5;
samples = 100;
dims = 4;
for sim = 1:1e3
    X = randn(dims, samples);
    [rootIndex,Tree] = kd_build(X);
    target = randn(dims,1);
    [nearest,index] = kd_search(rootIndex,Tree,target, k);
    dist = zeros(samples, 2);
    for i = 1:samples
        dist(i,1) = norm(X(:,i)-target);
        dist(i,2) = i;
    end
    dist = sortrows(dist);
    index1 = dist(1:k,2);
    if index ~= index1
        e = e+1;
    end
end
disp(['error cnt:', int2str(e)])


