#ubuntu下工程编译步骤
</br>
1.根据实际情况修改主makefile中的LIBPATH</br>

2.配置gcc的环境变量</br>

3.根据需要修改OPTIMIZE</br>

4.根据主makefile第84-94行及需要，选择一个配置选项进行make</br>
例如，现在需要编译stm32f103的工程，则执行： make stm32f103_config</br>

5.make</br>
