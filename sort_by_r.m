function X_sorted = sort_by_r(X,r)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 数据点集按照r维度排序
% X：数据点集[dim,num]
% r：排序维度
% X_sorted：排序后点集
% 简单冒泡排序
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% num = size(X,2);
% for i = 1:num-1
%     for j = 1:num-i
%         if X(r,j+1)<X(r,j)
%             X(:,[j,j+1]) = X(:,[j+1,j]);
%         end
%     end
% end
% X_sorted = X;
X_sorted = sortrows(X',r)';