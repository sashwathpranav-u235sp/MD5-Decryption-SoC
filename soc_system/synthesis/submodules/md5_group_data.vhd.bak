-- ==========================================
-- File				: md5_data.vhd
-- Description		: Avalon Memory Mapped Slave to Interface data of md5_unit.vhd
-- Author			: Timmy Huy Xuan Ngo 501031027
-- ==========================================

LIBRARY IEEE;
USE IEEE.std_logic_1164.ALL;
USE IEEE.numeric_std.all;

ENTITY md5_group_data IS
	PORT (
		avs_s0_address   	: IN  std_logic_vector(3 DOWNTO 0)  	:= (OTHERS => '0');	-- s0.address
		avs_s0_write     	: IN  std_logic                     	:= '0';             	-- s0.write
		avs_s0_writedata 	: IN  std_logic_vector(31 DOWNTO 0) 	:= (OTHERS => '0'); 	-- s0.writedata
		avs_s0_read      	: IN  std_logic                     	:= '0';             	-- s0.read
		avs_s0_readdata  	: OUT std_logic_vector(31 DOWNTO 0);                    		-- s0.readdata
		clk             	: IN  std_logic                     	:= '0';             	-- clock.clk
		reset           	: IN  std_logic                     	:= '0';             	-- reset.reset
		md5_writeaddr		: IN STD_LOGIC_VECTOR(8 DOWNTO 0)		:= (OTHERS => '0');
		md5_readaddr		: IN 	STD_LOGIC_VECTOR(6 DOWNTO 0)		:= (OTHERS => '0');
		md5_writedata     : IN std_logic_vector(31 DOWNTO 0)   	:= (OTHERS => '0');                		-- .md5_data
		md5_readdata		: OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
	);
END ENTITY md5_group_data;

ARCHITECTURE rtl OF md5_group_data IS
	
	SIGNAL readdata : STD_LOGIC_VECTOR(31 DOWNTO 0);
	
BEGIN
	PROCESS (clk, reset, avs_s0_read, avs_s0_write, avs_s0_address, avs_s0_writedata)
	BEGIN
		IF (reset = '1') THEN
			avs_s0_readdata <= (OTHERS => '0');
			md5_readdata <= (OTHERS => '0');
		ELSIF (rising_edge(clk)) THEN
			IF (avs_s0_read = '1') THEN
				CASE avs_s0_address IS
					WHEN "0000" =>
						avs_s0_readdata <= "00000000000000000000000"  & md5_writeaddr;
					WHEN "0001" =>
						avs_s0_readdata <= "000000000000000000000" & md5_readaddr;
					WHEN "0010" =>
						avs_s0_readdata <= md5_writedata;
					WHEN OTHERS =>
						avs_s0_readdata <= (OTHERS => '0');
				END CASE;
			ELSIF (avs_s0_write = '1') THEN
				CASE avs_s0_address IS
					WHEN "0000" =>
						readdata <= avs_s0_writedata;
					WHEN OTHERS =>
					
				END CASE;
			END IF;
		END IF;
	END PROCESS;
	
	md5_readdata <= readdata;
END ARCHITECTURE;