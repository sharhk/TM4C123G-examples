global s
delete(instrfindall);%删除现在设置的串口
s=serial('com5');% 根据电脑进行选择com
set(s,'BaudRate',9600,'Parity','none','DataBits',8,'StopBits',1);
s.InputBufferSize=4096;
s.OutputBufferSize=4096;
s.BytesAvailableFcnMode = 'byte';%串口接收中断配置，此步配置中断触发模式是根据字节
s.BytesAvailableFcnCount = 8;%收入八个字符可触发中断。
fopen(s);
SendReceive(s,10000000);
SendReceive(s,11111111);
SendReceive(s,10101010);
SendReceive(s,11010101);
%fclose(s);
%delete(s);     % 直接关掉的话影响接收，当然是在matlab串口只跑这点程序时使用

%中断函数
function callback(obj,~)
global s
  c = fread(s, 8);
  d=char(c')
end
%一组数据的收发函数
function SendReceive(s,number)
char=num2str(number);
fwrite(s,char,'uint8');
s.bytesAvailableFcn=@callback;
end 
