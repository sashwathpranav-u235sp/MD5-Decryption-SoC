-- ==========================================
-- File				: md5_data.vhd
-- Description		: Avalon Memory Mapped Slave to Interface data of md5_unit.vhd
-- Author			: Timmy Huy Xuan Ngo 501031027
-- ==========================================

LIBRARY IEEE;
USE IEEE.std_logic_1164.ALL;
USE IEEE.numeric_std.all;

ENTITY md5_data IS
	PORT (
		avs_s0_address   	: IN  std_logic_vector(3 DOWNTO 0)  	:= (OTHERS => '0');	-- s0.address
		avs_s0_write     	: IN  std_logic                     	:= '0';             	-- s0.write
		avs_s0_writedata 	: IN  std_logic_vector(31 DOWNTO 0) 	:= (OTHERS => '0'); 	-- s0.writedata
		avs_s0_read      	: IN  std_logic                     	:= '0';             	-- s0.read
		avs_s0_readdata  	: OUT std_logic_vector(31 DOWNTO 0);                    		-- s0.readdata
		clk             	: IN  std_logic                     	:= '0';             	-- clock.clk
		reset           	: IN  std_logic                     	:= '0';             	-- reset.reset
		md5_digest     	: IN  std_logic_vector(127 DOWNTO 0) 	:= (OTHERS => '0'); 	-- .md5_digest
		md5_data        	: OUT std_logic_vector(31 DOWNTO 0)                    		-- .md5_data
	);
END ENTITY md5_data;

ARCHITECTURE rtl OF md5_data IS
	
	SIGNAL data	: std_logic_vector(31 DOWNTO 0);
	
BEGIN
	PROCESS (clk, reset, avs_s0_read, avs_s0_write, avs_s0_address, avs_s0_writedata)
	BEGIN
		IF (reset = '1') THEN
			avs_s0_readdata <= (OTHERS => '0');
			md5_data <= (OTHERS => '0');
		ELSIF (rising_edge(clk)) THEN
			IF (avs_s0_read = '1') THEN
				CASE avs_s0_address IS
					WHEN "0000" =>
						avs_s0_readdata <= md5_digest(31 DOWNTO 0);
					WHEN "0001" =>
						avs_s0_readdata <= md5_digest(63 DOWNTO 32);
					WHEN "0010" =>
						avs_s0_readdata <= md5_digest(95 DOWNTO 64);
					WHEN "0011" =>
						avs_s0_readdata <= md5_digest(127 DOWNTO 96);
					WHEN "0100" =>
						avs_s0_readdata <= data;
					WHEN OTHERS =>
						avs_s0_readdata <= (OTHERS => '0');
				END CASE;
			ELSIF (avs_s0_write = '1') THEN
				CASE avs_s0_address IS
					WHEN "0000" =>
						data <= avs_s0_writedata;
					WHEN OTHERS =>
					
				END CASE;
			END IF;
		END IF;
	END PROCESS;
	
	md5_data <=  data;
END ARCHITECTURE;