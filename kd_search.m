function [nearest,nearestIndex] = kd_search(rootIndex,Tree,target, k)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 胡力
% 2018/8/6
% kd树的最邻近搜索
% rootIndex：数据点集建立的kd树的根节点下标(在树cell中的位置）
% target：目标点坐标
% Tree：kd树，cell类型
% nearst：点集中距离目标点最近的点下标
% nearst：点集中距离目标点最近的点坐标
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
currentNode = rootIndex; %当前节点下标，从根节点开始搜索
currentNode = search_down(Tree,currentNode,target);% 从currentNode向下搜索到底部
Tree{currentNode}.visited = 1;
    
currentNearest = Tree{currentNode}.val;                    % 当前最近点
currentNearestDist = norm(currentNearest-target);    % 当前最近距离
currentNearestIndex = currentNode;

kNearestCandidate = [currentNearestDist, currentNearest', currentNearestIndex];

while Tree{currentNode}.isRoot == 0
    isLeft = Tree{currentNode}.isLeft;     %当前节点是左孩子标志
    currentNode = Tree{Tree{currentNode}.parent}.index;
    if Tree{currentNode}.visited == 0
        Tree{currentNode}.visited = 1;%标记为已访问
        temp = norm(Tree{currentNode}.val-target);
        if temp<kNearestCandidate(end,1) || size(kNearestCandidate, 1)<k
            currentNearest = Tree{currentNode}.val;
            currentNearestDist = temp;
            currentNearestIndex = currentNode;
            % 加入，排序，末位&长尾淘汰
            kNearestCandidate = [kNearestCandidate; currentNearestDist, currentNearest', currentNearestIndex];
            kNearestCandidate = sortrows(kNearestCandidate);
            if size(kNearestCandidate)>=k
                kNearestCandidate(k+1:end,:) = [];
            end
        end
        temp = abs(Tree{currentNode}.val(Tree{currentNode}.r)-target(Tree{currentNode}.r));   %与当前分割线的距离
        if temp<kNearestCandidate(end,1) || size(kNearestCandidate, 1)<k
            %当前分割线距离小于当前最小距离，在分割线另一边可能有更近点，跳到另外一边继续搜索
            if isLeft == 1
                if Tree{currentNode}.hasRight == 1 %当前节点的左孩子，且当前节点有右孩子，则搜索右孩子
                    currentNode = Tree{Tree{currentNode}.right}.index;
                    currentNode = search_down(Tree,currentNode,target); 
                    Tree{currentNode}.visited = 1;%标记为已访问
                    temp = norm(target - Tree{currentNode}.val);
                    if temp<kNearestCandidate(end,1) || size(kNearestCandidate, 1)<k
                        currentNearest = Tree{currentNode}.val;
                        currentNearestDist = temp;
                        currentNearestIndex = currentNode;
                        kNearestCandidate = [kNearestCandidate; currentNearestDist, currentNearest', currentNearestIndex];
                        kNearestCandidate = sortrows(kNearestCandidate);
                        if size(kNearestCandidate)>=k
                            kNearestCandidate(k+1:end,:) = [];
                        end
                    end
                end
            else
                if Tree{currentNode}.hasLeft == 1  %当前节点是右孩子，且父亲节点有左孩子，则搜索父亲节点的左孩子
                    currentNode = Tree{Tree{currentNode}.left}.index;
                    currentNode = search_down(Tree,currentNode,target);
                    Tree{currentNode}.visited = 1;%标记为已访问
                    temp = norm(target - Tree{currentNode}.val);
                    if temp<kNearestCandidate(end,1) || size(kNearestCandidate, 1)<k
                        currentNearest = Tree{currentNode}.val;
                        currentNearestDist = temp;
                        currentNearestIndex = currentNode;
                        kNearestCandidate = [kNearestCandidate; currentNearestDist, currentNearest', currentNearestIndex];
                        kNearestCandidate = sortrows(kNearestCandidate);
                        if size(kNearestCandidate)>=k
                            kNearestCandidate(k+1:end,:) = [];
                        end
                    end
                end
            end
        end
    end
end
nearest = kNearestCandidate(:,2:end-1);
nearestIndex = kNearestCandidate(:,end);
            
        
            
        
