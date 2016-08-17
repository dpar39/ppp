function eye_separation_face_height_experiment()

root_dir = 'mugshot_frontal_original_all';
img_files = dir([root_dir '/*.jpg']);
pos_files = dir([root_dir '/*.pos']);
ann_files = dir([root_dir '/*.ann']);

do_display = 0; % Don't display annotations

num_faces = length(pos_files);
image_indices = 1:num_faces;
%image_indices = [98];
%image_indices = [61, 76, 88, 92, 107, 115];

face_height=zeros(num_faces,1);
eye_separation=zeros(num_faces,1);
chin=zeros(num_faces,1);

eye_angle = zeros(num_faces,1);
fac_angle = zeros(num_faces,1);

dl = zeros(num_faces,1);
dr = zeros(num_faces,1);

% Ratio positions respect to face box to estimate where to look for eyes
rlx = zeros(num_faces,1);
rly = zeros(num_faces,1);
rrx = zeros(num_faces,1);
rry = zeros(num_faces,1);

vlx = zeros(num_faces,1);
vly = zeros(num_faces,1);
vrx = zeros(num_faces,1);
vry = zeros(num_faces,1);

% Distance from eye centre to chin corner and head top
dc_chin=zeros(num_faces,1);
dc_crown=zeros(num_faces,1);
dc_mouth=zeros(num_faces,1);
mouth_width=zeros(num_faces,1);
sq_f_h = zeros(num_faces,1);

% Distance from eyes to mouth
dc_eye_mouth = zeros(num_faces,1);
dc_chin_crown = zeros(num_faces,1);

for k=1:length(image_indices),    
    i=image_indices(k);
    lm = load([root_dir '/' pos_files(i).name]);    
    rm = load([root_dir '/' ann_files(i).name]);
    
    rlx(i) = -(rm(1) - lm(7,1)) / rm(3);
    rly(i) = -(rm(2) - lm(7,2)) / rm(4);    
    rrx(i) = -(rm(1) - lm(10,1)) / rm(3);
    rry(i) = -(rm(2) - lm(10,2)) / rm(4);
    
    vlx(i) = -(rm(1) - rm(5)) / rm(3);
    vly(i) = -(rm(2) - rm(6)) / rm(4);    
    vrx(i) = -(rm(1) - rm(7)) / rm(3);
    vry(i) = -(rm(2) - rm(8)) / rm(4);
    
    face_height(i) = distance(lm(1,1), lm(1,2),lm(17,1), lm(17,2));
    eye_separation(i) = distance(lm(7,1), lm(7,2),lm(10,1), lm(10,2));
    chin(i)=distance((lm(7,1)+lm(10,1))/2, (lm(7,2)+lm(10,2))/2, lm(17,1), lm(17,2));
    
    dc_eye_mouth(i) = distance((lm(7,1)+lm(10,1))/2, (lm(7,2)+lm(10,2))/2, (lm(15,1)+lm(16,1))/2, (lm(15,2)+lm(16,2))/2);
    dc_chin_crown(i) = distance(lm(1,1), lm(1,2), lm(17,1), lm(17,2));
    
    sq_f_h(i) = rm(4);
    
    [~,~,dp,dc] = closest_point_perpendicular(lm(7,1), lm(7,2), lm(10,1), lm(10,2), lm(1,1), lm(1,2));
    dc_crown(i)=dc;
    [~,~,dp,dc] = closest_point_perpendicular(lm(7,1), lm(7,2), lm(10,1), lm(10,2), lm(17,1), lm(17,2));
    dc_chin(i)=dc;
    
    [~,~,dp,dc1] = closest_point_perpendicular(lm(7,1), lm(7,2), lm(10,1), lm(10,2), lm(15,1), lm(15,2));
    [~,~,dp,dc2] = closest_point_perpendicular(lm(7,1), lm(7,2), lm(10,1), lm(10,2), lm(15,1), lm(15,2));
    dc_mouth(i)=(dc1+dc2)/2;
    
    mouth_width(i) = distance(lm(15,1), lm(15,2), lm(16,1), lm(16,2));
    
    
    dy_face = lm(17,2)-lm(1,2);
    dy_eyes = lm(7,2)-lm(10,2);
    fac_angle(i) = asin(dy_face/face_height(i));
    eye_angle(i) = asin(dy_eyes/eye_separation(i));
    
    dl(i) = distance(lm(7,1),lm(7,2),rm(5),rm(6));
    dr(i) = distance(lm(10,1),lm(10,2),rm(7),rm(8));
    
    if (do_display)
        img = imread([root_dir '/' img_files(i).name]);
        imshow(img);
        hold on
        pp = lm([1,17, 7, 10],:);
        scatter(pp(:,1), pp(:,2));
        scatter(rm([5,7, 9, 11, 13, 15]), rm([6,8, 10, 12, 14, 16]), 'x');
        rectangle('Position', rm(1:4))
        hold off
    end
end
close all;

ddd = (dc_chin+dc_crown)./sq_f_h;
plot(ddd/ mean(ddd));

xxx = [rlx; 1-rrx];
search_x = min(xxx)
search_w = max(xxx)-min(xxx)
yyy = [rly;rry]; 
search_y = min(yyy)
search_h = max(yyy)-min(yyy)

figure;
subplot(221)
boxplot(rlx)
subplot(222)
boxplot(rly)
subplot(223)
boxplot(rrx)
subplot(224)
boxplot(rry)

figure;
subplot(211)
stem(dl);
subplot(212)
stem(dr);

%plot(eye_separation_face_height_ratio);

%plot((fac_angle-eye_angle)*180/pi);

function [x,y,dp, dc]=closest_point_perpendicular(x1,y1,x2,y2,xp,yp)
x0=(x1+x2)/2;
y0=(y1+y2)/2;

a = (x1-x2);
b = (y1-y2);
c = y0*(y2-y1)+x0*(x2-x1);
a2b2 = a*a+b*b;

x = (b*(b*xp-a*yp) - a*c)/a2b2;
y = (a*(-b*xp + a*yp)-b*c)/a2b2;
dp = distance(x,y,xp,yp);
dc = distance(x,y,x0,y0);


function [x,y,dp,da] = closest_point(ax, ay, bx, by, px, py)
a_to_px = px - ax;
a_to_py = py - ay;  
a_to_bx = bx - ax;
a_to_by = by - ay; 
atb2 = a_to_bx * a_to_bx + a_to_by * a_to_by;

atp_dot_atb = a_to_px * a_to_bx + a_to_py * a_to_by;
t = atp_dot_atb / atb2;  
x = ax + a_to_bx * t;
y = ay + a_to_by * t;

dp = distance(x,y,xp,yp);
da = distance(x,y,xa,ya);

function d = distance(x1 ,y1, x2, y2)
d = sqrt((x1-x2).^2 + (y1-y2).^2);

function stats_data(v)
m_v = mean(v);
s_v = std(v);
disp(m_v, s_v);
%sprintf('Mean=%f, Std