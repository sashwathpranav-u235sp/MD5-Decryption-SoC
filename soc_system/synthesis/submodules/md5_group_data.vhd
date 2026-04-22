-- ==========================================
-- File			: md5_group_data.vhd
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
		-- FIX: md5_writeaddr, md5_readaddr, md5_writedata changed from IN to OUT
		--      so this component drives the hardware, not the other way around.
		md5_writeaddr		: OUT STD_LOGIC_VECTOR(8 DOWNTO 0);
		md5_readaddr		: OUT STD_LOGIC_VECTOR(6 DOWNTO 0);
		md5_writedata     	: OUT std_logic_vector(31 DOWNTO 0);
		-- md5_readdata is IN: the digest result comes back from the hardware
		md5_readdata		: IN  STD_LOGIC_VECTOR(31 DOWNTO 0)		:= (OTHERS => '0')
	);
END ENTITY md5_group_data;

ARCHITECTURE rtl OF md5_group_data IS

	-- FIX: Internal registers to hold CPU-written values for all data-path signals.
	--      Previously only one register existed (readdata) and the write CASE only
	--      handled address 0, so writeaddr, readaddr, and wr were never set.
	SIGNAL reg_writedata : STD_LOGIC_VECTOR(31 DOWNTO 0) := (OTHERS => '0');
	SIGNAL reg_writeaddr : STD_LOGIC_VECTOR(8 DOWNTO 0)  := (OTHERS => '0');
	SIGNAL reg_readaddr  : STD_LOGIC_VECTOR(6 DOWNTO 0)  := (OTHERS => '0');
	SIGNAL reg_wr        : STD_LOGIC := '0';

BEGIN
	PROCESS (clk, reset)
	BEGIN
		IF (reset = '1') THEN
			avs_s0_readdata <= (OTHERS => '0');
			reg_writedata   <= (OTHERS => '0');
			reg_writeaddr   <= (OTHERS => '0');
			reg_readaddr    <= (OTHERS => '0');
			reg_wr          <= '0';
		ELSIF (rising_edge(clk)) THEN
			-- FIX: Write CASE now covers all four registers the C program writes:
			--   addr 0 -> writedata   (alt_write_word(md5_group_data+0, data))
			--   addr 1 -> writeaddr   (alt_write_word(md5_group_data+1, addr))
			--   addr 2 -> readaddr    (alt_write_word(md5_group_data+2, i))
			--   addr 3 -> wr flag     (alt_write_word(md5_group_data+3, 0x1 / 0x0))
			IF (avs_s0_write = '1') THEN
				CASE avs_s0_address IS
					WHEN "0000" =>
						reg_writedata <= avs_s0_writedata;
					WHEN "0001" =>
						reg_writeaddr <= avs_s0_writedata(8 DOWNTO 0);
					WHEN "0010" =>
						reg_readaddr <= avs_s0_writedata(6 DOWNTO 0);
					WHEN "0011" =>
						reg_wr <= avs_s0_writedata(0);
					WHEN OTHERS =>
						NULL;
				END CASE;
			-- FIX: Read CASE now returns the digest word at addr 0 (md5_readdata from
			--      hardware), and the wr status at addr 4 so the C poll loop terminates.
			--      Previously addr 0 returned md5_writeaddr (an input port), which was
			--      wrong, and there was no addr 4 case so the wr-ready poll hung on 0.
			ELSIF (avs_s0_read = '1') THEN
				CASE avs_s0_address IS
					WHEN "0000" =>
						avs_s0_readdata <= md5_readdata;
					WHEN "0100" =>
						avs_s0_readdata <= (31 DOWNTO 1 => '0') & reg_wr;
					WHEN OTHERS =>
						avs_s0_readdata <= (OTHERS => '0');
				END CASE;
			END IF;
		END IF;
	END PROCESS;

	-- Drive hardware outputs from internal registers
	md5_writedata <= reg_writedata;
	md5_writeaddr <= reg_writeaddr;
	md5_readaddr  <= reg_readaddr;

END ARCHITECTURE;