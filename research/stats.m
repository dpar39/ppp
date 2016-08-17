function stats( v )

m_v = mean(v);
s_v = std(v);

range = max(v)-min(v);
disp(['Mean=' num2str(m_v) ', Std='  num2str(s_v) '(' num2str(s_v*100/m_v) '%)']);
disp(['Range=' num2str(range) '[' num2str(min(v)) ',' num2str(max(v)) '], ' num2str(range*100/m_v) '%']);
disp(['MaxDev=' num2str(max(abs(max(v)-m_v), abs(m_v - min(v)))*100/m_v) '%']);
disp(['RangeCentre=' num2str((max(v)+min(v))/2) ' ['   num2str(range)  ']']);
end

