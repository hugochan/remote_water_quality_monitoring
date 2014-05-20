<h2>
<a name="remote_water_quality_monitoring" class="anchor" href="#remote_water_quality_monitoring"><span class="octicon octicon-link"></span></a>remote_water_quality_monitoring</h2>
<p>a lower computer program as a part of a project aimed at monitoring remote water quality...(stm32f4-based, gsm communication)</p>
<br><br>

<h2>
<a name="general-description" class="anchor" href="#general-description"><span class="octicon octicon-link"></span></a>1 ϵͳ������general description��</h2>

<h3>
<a name="system-framework" class="anchor" href="#system-framework"><span class="octicon octicon-link"></span></a>1.1 ϵͳ�ṹ��system framework��</h3>
<p>&nbsp;&nbsp;��ϵͳ��һ��<strong>��ʾϵͳ</strong>���ɿ���ģ�顢ͨ��ģ�顢������ģ�顢��Դģ�鹹�ɡ�����ģ�����΢��������Ϊ�������ģ�ͨ��ģ�����<strong>GSM</strong>ģ�飬������ģ��Ŀǰ�����û��ӿ�<strong>ģ��</strong>����ˮ�ʼ�����ݣ���Դģ��Ŀǰֱ��ʹ���ⲿ�����Դ��</p>
<br>

<h3>
<a name="system-function" class="anchor" href="#system-function"><span class="octicon octicon-link"></span></a>1.2 ϵͳ���ܼ�飨system function��</h3>
<p>&nbsp;&nbsp;��ϵͳ��"����GSM��ȡˮ��Զ�̹�����ˮ�ʾ���ϵͳ"��<strong>��λ��ϵͳ</strong>������<strong>���ϵͳ��װˮ��ˮ�ʣ���ʵʱ��������Ľ�����Ϣ</strong>�Ĺ��ܡ�����˵������ϵͳ�ܼ��PHֵ�������ȸ���ˮ�����ݣ���������ķ���������������Ϣ��������Ϣ�����������Ϣ������״̬��Ϣ�����ܿ������Ŀ���ָ�</p>
<br><br>

<h2>
<a name="running-environment" class="anchor" href="#running-environment"><span class="octicon octicon-link"></span></a>2 ���л�����running environment��</h2>

<h3>
<a name="physical-environment" class="anchor" href="#physical-environment"><span class="octicon octicon-link"></span></a>2.1 ��������physical environment��</h3>
<p>&nbsp;&nbsp;��ϵͳδ����������ˮ�⡢������Ұ�⻷������Ŀǰ����ʾ�汾�����ڽ��ܡ�������ȷ���δ�ﵽ��ҵ��׼��</p>
<br>

<h3>
<a name="hardware" class="anchor" href="#hardware"><span class="octicon octicon-link"></span></a>2.2 Ӳ���豸��hardware��</h3>
<p>&nbsp;&nbsp;����Cortex-M4�ں˵�32λ΢������<strong>STM32F4</strong>��Ϊ��λ��ϵͳ����ƽ̨���Ի�Ϊ��˾��<strong>GTM900</strong>Ϊ���ĴGSMģ�顣����λ��ϵͳ��Ϊһ��ͨ����ʾƽ̨��Ŀǰδ�����κδ�����ģ�飬���ṩģ�����ݿ��ƽӿڣ���Ϊ�����պ�ʵ��Ͷ����������չ��Ҫ��ϵͳ֧�ִ�����ģ����չ��</p>
<br>

<h3>
<a name="software" class="anchor" href="#software"><span class="octicon octicon-link"></span></a>2.2 �����ͨ��Э�飩֧�֣�software&protocol��</h3>
<p>&nbsp;&nbsp;����GSM��Ϊ�ײ�ͨ��֧�֣�ʹ�ö�����Ϊ���ݽ���ý�飩��Ӧ�ò�������п�����ͨ��Э����Ϊ��λ��������������ݽ�������Ϊ�淶��</p>
<p>&nbsp;&nbsp;��λ��ϵͳ��������Ľ������¼�����Ϣ��</p>

<ul class="task-list">
	<li>��1��sj�����ݣ���Ϣ�����š�PHֵ������ֵ��ˮ��״̬����ǰʱ�䡣</li>
	<li>��2��zt��״̬����Ϣ�����������š�PHֵ������ֵ��ˮ��״̬����ǰʱ�䡣</li>
	<li>��3��kz (����) ��Ϣ�����������š�����ˮ��״̬����ǰʱ�䡣</li>
	<li>��4��jb����������Ϣ�����������š�PHֵ��ˮ��״̬����ǰʱ�䡣</li>
	<li>��5��nj�������������Ϣ�����������š�PHֵ��ˮ��״̬����ǰʱ�䡣</li>
	<li>��6��sjack������Ӧ����Ϣ�����������š���ǰʱ�䡣</li>
	<li>��7��ztqry��״̬ѯ�ʣ���Ϣ�����������š���ǰʱ�䡣</li>
	<li>��8��jback������Ӧ����Ϣ�����������š���ǰʱ�䡣</li>	
	<li>��9��njack���������Ӧ����Ϣ�����������š���ǰʱ�䡣</li>
</ul>
<br><br>

<h2>
<a name="how-to-use" class="anchor" href="#how-to-use"><span class="octicon octicon-link"></span></a>3 ϵͳ����˵����how to use��</h2>

<h3>
<a name="install-running" class="anchor" href="#install-running"><span class="octicon octicon-link"></span></a>3.1 ��װ�����У�install&running��</h3>

<h4>
<a name="install" class="anchor" href="#install"><span class="octicon octicon-link"></span></a>3.1.1 ��װ��install��</h4>
<p>&nbsp;&nbsp;����λ����������Ϣ�������š��������ĺ���̨��Ӧ��ͨ��Э��ȣ�������д��̼��У�ϵͳ�ϵ缴�����С���д������IAR��������������ɡ�</p>

<h4>
<a name="running" class="anchor" href="#running"><span class="octicon octicon-link"></span></a>3.1.2 ���У�running��</h4>
<p>&nbsp;&nbsp;����λ��ϵͳ�������Զ����ν������³�ʼ��������</p>
<ul class="task-list">
	<li>��1������������Ϣ����ʼ������Դ����ʱ����GSMģ�顢��������ʼ���ȣ���</li>
  <li>��2����������ģ���λ��ϵͳ��ע���豸��Ϣ��ʵ������sj��Ϣ����</li>
  <li>��3���ȴ��������ĵ�ע��ظ���Ϣ����λ������ʱ�ط����ܣ���ϵͳ��ʼ����Ϊ60s����ע��ظ���Ϣ����������Ӧ����Ϣ����״̬ѯ����Ϣ������״̬ѯ����Ϣ����λ���Զ��ظ���ǰ״̬��ע����ɡ�</li>
  <li>��4��������������״̬��</li>
</ul>
<p>&nbsp;&nbsp;��������״̬�£��Զ�ִ�����²�����</p>
<ul class="task-list">
	<li>��1����ʱ��������ķ���sj����Ϣ��</li>
	<li>��2�������λ�����ư�ť������ϵͳ��ʾ�ã���������ph/������/����ť��ϵͳģ���ph/����ֵ��/��1����λ��ָʾ��ָʾ���ݱ仯������������ť�����������ģʽ����Ч����������ر�ˮ����״ָ̬ʾ��ָʾ����������������������ķ���jb����������Ϣ����ʱ�ط�ʱ���ʼ����Ϊ60s�����������������ť��������ģʽ����Ч�����򾯱������״ָ̬ʾ��ָʾ����������ϵͳ�Զ���������ķ���nj�������������Ϣ����ʱ�ط�ʱ���ʼ����Ϊ60s����</li>
	<li>��3����ѯ�������Ŀ�����Ϣ����Щ������Ϣ����sjack������Ӧ����Ϣ��ztqry��״̬ѯ�ʣ���Ϣ��jback������Ӧ����Ϣ��njack���������Ӧ����Ϣ��kz�����ƣ���Ϣ������ѯ��ztqry��Ϣ�����Զ���������Ļظ���ǰzt��״̬����Ϣ������ѯ��jback��Ϣ����ֹͣjb��ʱ�ش���ʱ��������ѯ��njack��Ϣ����ֹͣnj��ʱ�ش���ʱ��������ѯ��kz��Ϣ�����Զ���������״̬��ˮ�����գ���ʾϵͳ��״ָ̬ʾ��ָʾ�����Զ���������Ļظ���ǰzt��״̬����Ϣ���յ�sjack������Ӧ����Ϣ��������</li>
</ul>
<br>

<h3>
<a name="exception-recovery" class="anchor" href="#exception-recovery"><span class="octicon octicon-link"></span></a>3.2 ��������ָ���exception&recovery��</h3>
<ul class="task-list">
	<li>��1��ϵͳ��ʼ���׶Σ�GSMģ������޷������������������ƶ�̨ע��sim����Ϣ������ϵͳ��ʼ�մ��ڳ�������״̬��</li>
	<li>��2��ϵͳ������������״̬������⵽GSMģ����ֹ��ϣ������ԭ�����޷������շ���Ϣ�������Զ��˳���������״̬��ת������GSMģ���ʼ��������Ȼ��Ϊ�Լ����ߣ���</li>
</ul>
<br>

<h3>
<a name="notice" class="anchor" href="#notice"><span class="octicon octicon-link"></span></a>3.3 ע�����notice��</h3>
<ul class="task-list">
	<li>��1����λ�������洢Ҳ����sim���Ķ������Ĵ洢���š����꼴ɾ���������Ķ��š�</li>
	<li>��2��������Ϣ������ʱ���⣬δ��������ϵͳ(����λ��)��Ϣӵ������λ����Ϣ��ʱ�ط������������ޣ���ʼ����Ϊ3����</li>
	<li>��3����λ��һ�����ߣ����������ע�ᣩ������Ϊ�Լ�ʼ�����ߣ��Ҳ����������ߣ�����������һ��ʱ����δ�õ���λ����Ϣ����������λ����ʶΪ����״̬��</li>
</ul>
<br><br>

<h2>
<a name="presentation" class="anchor" href="#presentation"><span class="octicon octicon-link"></span></a>4 ͼƬչʾ(presentation)</h2>
<h3>
	<p>��λ��ϵͳ��lower computer system��</p></h3>
&nbsp;&nbsp;&nbsp;&nbsp;<img src="./image/��λ��ϵͳ.png" height="300" width="500" alt="��λ��ϵͳ"></img>
<br><br><br><br>

<h3>
	<p>��λ��ϵͳ��upper computer system��</p></h3>
&nbsp;&nbsp;&nbsp;&nbsp;<img src="./image/��λ��ϵͳ.png" height="300" width="500" alt="��λ��ϵͳ"></img>
<br><br><br><br>