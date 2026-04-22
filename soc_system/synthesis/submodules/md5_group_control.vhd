-- ==========================================
-- File			: md5_group_control.vhd
-- Description		: Avalon Memory Mapped Slave to Interface controls of md5_unit.vhd
-- Author			: Timmy Huy Xuan Ngo 501031027
-- ==========================================

LIBRARY IEEE;
USE IEEE.std_logic_1164.ALL;
USE IEEE.numeric_std.ALL;

ENTITY md5_group_control IS
	PORT (
		avs_s0_address   	: IN  std_logic_vector(3 DOWNTO 0)  	:= (OTHERS => '0');	-- s0.address
		avs_s0_write     	: IN  std_logic                     	:= '0';             	-- s0.write
		avs_s0_writedata 	: IN  std_logic_vector(31 DOWNTO 0) 	:= (OTHERS => '0'); 	-- s0.writedata
		avs_s0_read      	: IN  std_logic                     	:= '0';             	-- s0.read
		avs_s0_readdata  	: OUT std_logic_vector(31 DOWNTO 0);                    		-- s0.readdata
		clk              	: IN  std_logic                     	:= '0';             	-- clock.clk
		reset            	: IN  std_logic                     	:= '0';             	-- reset.reset
		md5_wr				: IN  STD_logic						:= '0';					-- .md5_wr
		md5_start       	: OUT std_logic_vector(31 DOWNTO 0);                   		-- .md5_start
		md5_reset       	: OUT std_logic_vector(31 DOWNTO 0);                    		-- .md5_reset
		md5_done         	: IN  std_logic_vector(31 DOWNTO 0) 	:= (OTHERS => '0')  	-- .md5_done
	);
END ENTITY md5_group_control;

ARCHITECTURE rtl OF md5_group_control IS

	SIGNAL start, md5_reset_temp : std_logic_vector(31 DOWNTO 0);

BEGIN
	-- FIX: Removed md5_wr from the sensitivity list. md5_wr is a data-path
	--      handshake for the message RAM; it must not gate writes to the
	--      control registers. Previously the IF (md5_wr = '1') guard meant
	--      that alt_write_word(md5_group_control, 0x1) and
	--      alt_write_word(md5_group_control+1, ...) were silently discarded
	--      whenever md5_wr was low, so the MD5 engines never received a
	--      start or reset pulse.
	PROCESS (clk, reset)
	BEGIN
		IF (reset = '1') THEN
			start          <= (OTHERS => '0');
			md5_reset_temp <= (OTHERS => '0');
		ELSIF (rising_edge(clk)) THEN
			IF (avs_s0_write = '1') THEN
				-- FIX: Write CASE is now unconditional (md5_wr guard removed).
				CASE avs_s0_address IS
					WHEN "0000" =>
						start <= avs_s0_writedata;
					WHEN "0001" =>
						md5_reset_temp <= avs_s0_writedata;
					WHEN OTHERS =>
						NULL;
				END CASE;
			ELSIF (avs_s0_read = '1') THEN
				CASE avs_s0_address IS
					WHEN "0000" =>
						avs_s0_readdata <= start;
					WHEN "0001" =>
						avs_s0_readdata <= md5_reset_temp;
					WHEN "0010" =>
						avs_s0_readdata <= md5_done;
					WHEN OTHERS =>
						avs_s0_readdata <= (OTHERS => '0');
				END CASE;
			END IF;
		END IF;
	END PROCESS;

	md5_start <= start;
	md5_reset <= md5_reset_temp;

END ARCHITECTURE rtl;