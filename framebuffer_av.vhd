library ieee;
use ieee.std_logic_1164.all;

entity framebuffer_av is
port(
	-- Global signals
	clk : in std_logic;
	rst_n : in std_logic;
	
	-- VGA CTRL signals
	next_line : in std_logic;
	
	-- VRAM signals
	wr_addr : out std_logic_vector(7 downto 0);
	q : out std_logic_vector(15 downto 0);
	wren : out std_logic;
	
	--Avalon master signals
	address : out std_logic_vector(31 downto 0);
	read : out std_logic;
	flush : out std_logic;
	waitrequest : in std_logic;
	datavalid : in std_logic;
	readdata : in std_logic_vector(15 downto 0)
);
end entity;

architecture rtl of framebuffer_av is

component framebuffer is
port(
	-- Global signals
	clk : in std_logic;
	rst_n : in std_logic;
	
	-- VGA CTRL signals
	next_line : in std_logic;
	
	-- VRAM signals
	wr_addr : out std_logic_vector(7 downto 0);
	q : out std_logic_vector(15 downto 0);
	wren : out std_logic;
	
	--Avalon master signals
	address : out std_logic_vector(31 downto 0);
	read : out std_logic;
	flush : out std_logic;
	waitrequest : in std_logic;
	datavalid : in std_logic;
	readdata : in std_logic_vector(15 downto 0)
);
end component;

begin

framebuffer_0 : framebuffer
port map(
	clk => clk,
	rst_n => rst_n,
	next_line => next_line,
	wr_addr => wr_addr,
	q => q,
	wren => wren,
	address => address,
	read => read,
	flush => flush,
	waitrequest => waitrequest,
	datavalid => datavalid,
	readdata => readdata
);

end architecture;