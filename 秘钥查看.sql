select * from SECKEYINFO;
select * from keysn;

delete from SECKEYINFO where state=0;
update keysn set ikeysn=1;
commit;
