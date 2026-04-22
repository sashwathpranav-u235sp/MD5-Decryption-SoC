

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