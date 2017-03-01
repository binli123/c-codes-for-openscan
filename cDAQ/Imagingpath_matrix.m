%% parameters definition
nxAcq=512;
nx=1000;
ny=1000;
nTurn=nx-nxAcq;
nSettle=0;

%%generate imaging path
xxFillfrac=NaN(1,nTurn/2);
xxLine = [linspace(-0.5,0.5,nxAcq) xxFillfrac xxFillfrac];          %position array of per line
xx = repmat(xxLine(:),1,ny);  %matrix of whole x positions
yy=linspace(-0.5,0.5,(nx*ny-nTurn))';
yy=[zeros(nTurn/2,1);yy;ones(nTurn/2,1)];
slopeX=1/nxAcq;

splineInterp=spline([0 nTurn-nSettle+1],[slopeX 0.5, -nSettle*slopeX-0.5,slopeX],1:(nTurn-nSettle));      %x position info of each turn
settleInterp=linspace(-nSettle*slopeX-0.5,-slopeX-0.5,nSettle);                                             %x position info of each settle 

turnX=[splineInterp settleInterp];                              %x position info between two Acq

slopeY=0;
turnY=spline([0 nTurn+1],[slopeY,0,1/(ny-1),slopeY],1:nTurn);   %y position info between two Acq

%%transform meshgrid into column and replace Fillfrac with turnX and turnY
xx=reshape(xx,[],1);
yy=reshape(yy,[],1);
for line =1:(ny-1)
    startIdx=line*nx-nTurn+1;
    endIdx  =line*nx;
    xx(startIdx:endIdx)=turnX;
end
xxPerline=xx(1:1000);
yyPerline=xx(1:1000);
fileIDX = fopen('xxPerline.txt','w');
fprintf(fileIDX,'%6.3lf\n',xxPerline);
fclose(fileIDX);
fileIDY = fopen('yyPerling.txt','w');
fprintf(fileIDY,'%6.3lf\n',yyPerline);
fclose(fileIDY);


fileIDX = fopen('xx.txt','w');
fprintf(fileIDX,'%6.3f\n',xx);
fclose(fileIDX);
t=load('xx.txt');
plot(linspace(1,3000,3000)',xx(1:3000));

